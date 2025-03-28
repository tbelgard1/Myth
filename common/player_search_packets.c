#include "cseries.h"
#include "byte_swapping.h"
#include "metaserver_common_structs.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "users.h"
#include "room_packets.h"
#include "list_element.h"
#include "ps_query.h"
#include "player_search_packets.h"

static short build_empty_header(char * buffer, short type);
static short append_data_to_packet(char * buffer, void * data, short data_length);
static short build_empty_ps_query_response_header(char * buffer, long user_id);
static void byte_swap_query_response_packet(char * buffer, boolean outgoing);

short build_ps_login_packet(char * buffer, long room_id)
{
	build_empty_header(buffer, _ps_login_packet);
	return append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
}

short build_ps_add_player_packet(char * buffer, long room_id, long player_id, short order, struct metaserver_player_aux_data * player_aux_data, char * player_data)
{
	build_empty_header(buffer, _ps_add_player_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)&order, sizeof(order));
	append_data_to_packet(buffer, (void *)&order, sizeof(order));
	append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));	
	append_data_to_packet(buffer, (void *)player_aux_data, sizeof(struct metaserver_player_aux_data));
	return append_data_to_packet(buffer, (void *)player_data, player_aux_data->player_data_length);
}

short build_ps_delete_player_packet(char * buffer, long player_id, long room_id)
{
	build_empty_header(buffer, _ps_delete_player_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
}

short build_ps_query_packet(char * buffer, long player_id, long * buddy_list, short order, char * search_string)
{
	short unused;
	assert(search_string);

	build_empty_header(buffer, _ps_query_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)buddy_list, sizeof(long) * 8);
	append_data_to_packet(buffer, (void *)&order, sizeof(order));
	append_data_to_packet(buffer, (void *)&unused, sizeof(unused));
	return append_data_to_packet(buffer, (void *)search_string, strlen(search_string) + 1);
}

short build_ps_query_response_packet(char * buffer, short type, long user_id, struct query_response * qr)
{
	struct ps_query_response_segment * qs;
	struct ps_query_response_header * qh;
	struct room_packet_header * header;
	int string_len;
	int num_responses = 0;
	char * p_src, * p_dest, * p_segment;

	build_empty_ps_query_response_header(buffer, user_id);
	header = (struct room_packet_header *)buffer;
	p_segment = (char *)buffer;
	p_segment += sizeof(struct room_packet_header);
	qh = (struct ps_query_response_header *)p_segment;
	qh->type = type;

	p_segment += sizeof(struct ps_query_response_header);
	if (qr)
	{
		while (qr->match_score && num_responses < MAX_PLAYER_SEARCH_RESPONSES)
		{
			if (qr->element.player_id == user_id)
			{
				qr++;
				continue;
			}

			qs = (struct ps_query_response_segment *)p_segment;
			qs->room_id = qr->element.room_id;
			qs->player_aux_data = qr->element.player_aux_data;

			p_dest = (char *)qs;
			p_dest += sizeof(struct ps_query_response_segment);
			header->length += sizeof(struct ps_query_response_segment);

			p_src = (char *)&qr->element;
			// increment source past datum, room_id, and player_id, order, unused short, buddies, and metaserver_player_aux_data
			p_src += 48 + sizeof(struct metaserver_player_aux_data);			
			
			// copy all of the data before the strings, i.e. char + char + 9 shorts + char + char + 8 shorts
			memcpy(p_dest, p_src, 38);
			p_dest += 38;
			qs->player_aux_data.player_data_length = 38;
			
			strcpy(p_dest, qr->element.name);
			string_len = strlen(qr->element.name) + 1;
			p_dest += string_len;
			qs->player_aux_data.player_data_length += string_len;

			strcpy(p_dest, qr->element.team);
			string_len = strlen(qr->element.team) + 1;
			qs->player_aux_data.player_data_length += string_len;

			header->length += qs->player_aux_data.player_data_length;
			p_segment += qs->player_aux_data.player_data_length + sizeof(struct ps_query_response_segment);

			qr++;
			num_responses++;
		}
	}

	qh->number_of_responses = num_responses;

	return ((struct room_packet_header *)(buffer))->length;
}

static short build_empty_ps_query_response_header(char * buffer, long user_id)
{
	struct room_packet_header * header = (struct room_packet_header *)buffer;
	int n = 0;

	header->type = _ps_query_response_packet;
	header->length = sizeof(struct room_packet_header);
	append_data_to_packet(buffer, (void *)&user_id, sizeof(user_id));
	return append_data_to_packet(buffer, (void *)&n, sizeof(n));
}

boolean send_player_search_packet(int socket, char * buffer, short length)
{
	int last_sent;
	int left_to_send;
	char * index = buffer;

	byte_swap_player_search_packet(buffer, TRUE);
	left_to_send = length;
	while (left_to_send)
	{
		last_sent = send(socket, index, length, 0);
		if (last_sent == -1)
			return FALSE;
		left_to_send -= last_sent;
		index += last_sent;
	}

	byte_swap_player_search_packet(buffer, FALSE);
}

static byte_swap_code _bs_ps_query_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,			// player_id
		_4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte,	// buddy list
		_2byte, _2byte, // order, unused
	_end_bs_array
};

static byte_swap_code _bs_ps_login_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_ps_header_only[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_ps_query_response_header_only[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_ps_add_player_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,			// header
		_4byte, _2byte,			// player_id / order
		_2byte, _4byte,			// unused , room_id
		_2byte, _2byte,			// player_aux_data
		_4byte, _4byte,
		_4byte, _2byte, 
		_2byte,
	_end_bs_array
};

static byte_swap_code _bs_ps_delete_player_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_ps_query_response_segment[] =
{
	_begin_bs_array, 1,
		_4byte,					// room_id
		_2byte, _2byte,			// player_aux_data
		_4byte, _4byte,
		_4byte, _2byte, 
		_2byte,
	_end_bs_array
};

static P_BS_DATA bs_data[] = 
{
	_bs_ps_login_packet, 8,
	_bs_ps_add_player_packet, 16,
	_bs_ps_delete_player_packet, 12, 
	_bs_ps_query_packet, 8
};

static P_BS_DATA bs_data_header = 
{
	_bs_ps_header_only, 4
};

static P_BS_DATA bs_ps_query_response_segment = 
{
	_bs_ps_query_response_segment, 24
};

static P_BS_DATA bs_ps_query_response_header = 
{
	_bs_ps_query_response_header_only, 8
};

static void byte_swap_query_response_packet(char * buffer, boolean outgoing)
{
	char * byte_swap_me;
	int segment_index, number_of_responses;
	struct ps_query_response_header * ps_header;

	byte_swap_me = buffer;
	byte_swap_data("query response packet", byte_swap_me, bs_data_header.length, 1, bs_data_header.code);
	byte_swap_me += sizeof(struct room_packet_header);

	if (outgoing)
	{
		ps_header = (struct ps_query_response_header *)byte_swap_me;
		number_of_responses = ps_header->number_of_responses;
	}

	byte_swap_data("player search packet", byte_swap_me, bs_ps_query_response_header.length, 1, bs_ps_query_response_header.code);

	if (!outgoing)
	{
		ps_header = (struct ps_query_response_header *)byte_swap_me;
		number_of_responses = ps_header->number_of_responses;
	}

	byte_swap_me += sizeof(struct ps_query_response_header);

	// swap segments
	for (segment_index = 0; segment_index < number_of_responses; segment_index++)
	{
		int segment_length;

		if (outgoing)
			segment_length = ((struct ps_query_response_segment *)(byte_swap_me))->player_aux_data.player_data_length + sizeof(struct ps_query_response_segment);
		byte_swap_data("game search packet", byte_swap_me, bs_ps_query_response_segment.length, 1, bs_ps_query_response_segment.code);
		if (!outgoing)
			segment_length = ((struct ps_query_response_segment *)(byte_swap_me))->player_aux_data.player_data_length + sizeof(struct ps_query_response_segment);
		byte_swap_me += segment_length;
	}
}

boolean byte_swap_player_search_packet(char * buffer, boolean outgoing)
{
	short type, length;
	struct room_packet_header * header;
	boolean success;

	header = (struct room_packet_header *)buffer;
	type = header->type;
	length = header->length;

	if (!outgoing)
	{
		type = SWAP2(type);
		length = SWAP2(length);
	}

	if (type == _ps_query_response_packet)
		byte_swap_query_response_packet(buffer, outgoing);
	else
		byte_swap_data("player search packet", buffer, bs_data[type].length, 1, bs_data[type].code);

	switch (type)
	{
	case _ps_login_packet:
	case _ps_add_player_packet:
	case _ps_delete_player_packet:
	case _ps_query_packet:
	case _ps_query_response_packet:
		success = TRUE;
		break;
	default:
		success = FALSE;
		break;
	}

	return success;
}

static short build_empty_header(char * buffer, short type)
{
	struct room_packet_header * header = (struct room_packet_header *)buffer;

	header->type = type;
	header->length = sizeof(struct room_packet_header);

	return header->length;
}

static short append_data_to_packet(char * buffer, void * data, short length)
{
	struct room_packet_header * header = (struct room_packet_header *)buffer;
	char * dest = buffer + header->length;

	memcpy(dest, data, length);
	header->length += length;

	return header->length;
}

