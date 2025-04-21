/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

 /*
  * The metaserver code changes that fall outside the original Bungie.net metaserver code 
  * license were written and are copyright 2002, 2003 of the following individuals:
  *
  * Copyright (c) 2002, 2003 Alan Wagner
  * Copyright (c) 2002 Vishvananda Ishaya
  * Copyright (c) 2003 Bill Keirstead
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
  */

#include "cseries.h"
#include "byte_swapping.h"
#include "metaserver_common_structs.h"
#include "authentication.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "rank.h"
#include "games.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "game_search_packets.h"

// ALAN Begin: added headers
#include <string.h>
#include <sys/socket.h>
// ALAN End

static short build_empty_header(char *buffer, short type);
static short append_data_to_packet(char *buffer, void *data, short data_length);

static short build_empty_gs_query_response_packet_header(char *buffer, unsigned long user_id);
static short append_segment_to_gs_query_response_packet(char *buffer, struct gs_query_response_segment *seg);

void byte_swap_query_response_packet(char *buffer, boolean outgoing);

short build_gs_login_packet(
	char *buffer, 
	long room_id)
{
	build_empty_header(buffer, _gs_login_packet);
	return append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
}

short build_gs_update_packet(
	char *buffer, 
	int type, 
	long room_id, 
	boolean game_is_ranked, 
	struct metaserver_game_aux_data *aux_data, 
	int game_data_length, 
	struct metaserver_game_description *game)
{
	build_empty_header(buffer, _gs_update_packet);
	append_data_to_packet(buffer, (void *)&type, sizeof(type));
	append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
	append_data_to_packet(buffer, (void *)&game_is_ranked, sizeof(game_is_ranked));
	append_data_to_packet(buffer, (void *)aux_data, sizeof(struct metaserver_game_aux_data));
	append_data_to_packet(buffer, (void *)&game_data_length, sizeof(game_data_length));
	return append_data_to_packet(buffer, (void *)game, game_data_length);
}

short build_gs_update_delete_packet(
	char *buffer, 
	int type, 
	long game_id, 
	long room_id,
	long creating_player_id)
{
	struct metaserver_game_aux_data aux_data;

	memset(&aux_data, 0, sizeof(aux_data));	
	aux_data.creating_player_id= creating_player_id;
	build_empty_header(buffer, _gs_update_packet);
	append_data_to_packet(buffer, (void *)&type, sizeof(type));
	append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
	append_data_to_packet(buffer, (void *)&game_id, sizeof(game_id));
	append_data_to_packet(buffer, (void *)&aux_data, sizeof(struct metaserver_game_aux_data));
	return append_data_to_packet(buffer, (void *)&game_id, sizeof(game_id));
}

short build_gs_query_packet(
	char *buffer,
	unsigned long player_id, 
	char *game_name, 
	char *map_name, 
	short game_scoring, 
	short unit_trading, 
	short veterans, 
	short teams, 
	short alliances, 
	short enemy_visibility)
{
	// ALAN Begin: unused variable
//	unsigned long flags= 0l;
	// ALAN End

	build_empty_header(buffer, _gs_query_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)&game_scoring, sizeof(game_scoring));
	append_data_to_packet(buffer, (void *)&game_scoring, sizeof(game_scoring));
	append_data_to_packet(buffer, (void *)&unit_trading, sizeof(unit_trading));
	append_data_to_packet(buffer, (void *)&veterans, sizeof(veterans));
	append_data_to_packet(buffer, (void *)&teams, sizeof(teams));
	append_data_to_packet(buffer, (void *)&alliances, sizeof(alliances));
	append_data_to_packet(buffer, (void *)&enemy_visibility, sizeof(enemy_visibility));
	append_data_to_packet(buffer, (void *)&enemy_visibility, sizeof(enemy_visibility));
	append_data_to_packet(buffer, (void *)game_name, strlen(game_name) + 1);
	return append_data_to_packet(buffer, (void *)map_name, strlen(map_name) + 1);
}

int build_gs_query_response_packet(
	char *buffer,
	unsigned long user_id, 
	struct query_response *qr)
{
	char scratch[4096];
	struct gs_query_response_segment *gs_segment = (struct gs_query_response_segment *)scratch;
	char *text_dest;
	int string_len;

	build_empty_gs_query_response_packet_header(buffer, user_id);

	while (qr->data_index != DATA_INDEX_UNUSED)
	{
		gs_segment->room_id = qr->room_id;
		gs_segment->game_is_ranked = qr->game_is_ranked;
		gs_segment->aux_data = qr->aux_data;
		gs_segment->game_data_length = qr->game_data_length;
		gs_segment->game = qr->game;

		// ALAN Begin: assignment from incompatible pointer type fix
//		text_dest = &gs_segment->game;
		text_dest = (char *)&gs_segment->game;
		// ALAN End

		text_dest+= sizeof(struct metaserver_game_description);
		strcpy(text_dest, qr->game_name);
		string_len = strlen(qr->game_name)+1;
		text_dest+= string_len;
		gs_segment->segment_length= sizeof(struct gs_query_response_segment)+string_len;
		strcpy(text_dest, qr->map_name);
		string_len = strlen(qr->map_name) + 1;
		gs_segment->segment_length+= string_len;

		append_segment_to_gs_query_response_packet(buffer, gs_segment);
		qr++; 
	}

	return ((struct room_packet_header *)(buffer))->length;
}

boolean send_game_search_packet(
	int socket, 
	char *buffer, 
	short length)
{
	int last_sent;
	int left_to_send;
	char *index= buffer;
	
	byte_swap_game_search_packet(buffer, TRUE);
	left_to_send= length;
	while (left_to_send)
	{
		last_sent = send(socket, index, length, 0);
		if (last_sent==-1) return FALSE;
		left_to_send-= last_sent;
		index+= last_sent;		
	}

	byte_swap_game_search_packet(buffer, FALSE);
	
	return TRUE;
}

static byte_swap_code _bs_gs_header_only[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_gs_query_response_header_only[] =
{
	_begin_bs_array, 1,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_gs_login_packet[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_gs_update_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,		// header
		_4byte,			// type
		_4byte,			// room_id
		_4byte,			// game_is_ranked
		_4byte, _4byte, _2byte, 1,
		1, _4byte, _4byte, _2byte, 
		_2byte, _4byte, _4byte,	// metaserver_game_aux_data
		_4byte,			// game description length
		_2byte, _2byte, _4byte, _4byte,
		_4byte, _2byte, _2byte, _2byte,
		_2byte, _4byte, _4byte, _4byte,
		_4byte,	_2byte, 512,		// game description parameters
		_4byte,			// game description public_tags checksum
		_4byte,			// game description private_tags checksum
		_2byte,			// game description flags
		1,			// game description player count
	_end_bs_array
};

static byte_swap_code _bs_gs_query_response_segment[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte,		// header
		_4byte,			// n_segment_length
		_4byte,			// roomd id
		_4byte,			// game is ranked
		_4byte, _4byte, _2byte, 1,
		1, _4byte, _4byte, _2byte, 
		_2byte, _4byte, _4byte,	// metaserver_game_aux_data
		_4byte, 		// game description length
		_2byte, _2byte, _4byte, _4byte,
		_4byte, _2byte, _2byte, _2byte,
		_2byte, _4byte, _4byte, _4byte,
		_4byte,	_2byte, 512,	// game description parameters
		_4byte,			// game description public_tags checksum
		_4byte,			// game description private_tags checksum
		_2byte,			// game description flags
		1,			// game description player count
	_end_bs_array
};

static byte_swap_code _bs_gs_query_packet[] =
{
	_begin_bs_array, 1,
		_2byte, _2byte,		// header
		_4byte,				// player_id
		_2byte, _2byte,		// game_type / scoring
		_2byte, _2byte, _2byte, _2byte,
		_2byte, _2byte,
	_end_bs_array
};

/* ALAN Begin: defined but not used
static byte_swap_code _bs_game_aux_data[]= 
{
	_begin_bs_array, 1,
		_4byte, _4byte, _2byte, 1,
		1, _4byte, _4byte, _2byte, 
		_2byte, _4byte, _4byte,	// metaserver_game_aux_data
	_end_bs_array
};
// ALAN End */

static byte_swap_code _bs_game_data[] = 
{
	_begin_bs_array, 1,
		_2byte, _2byte, _4byte, _4byte,
		_4byte, _2byte, _2byte, _2byte,
		_2byte, _4byte, _4byte, _4byte,
		_4byte,	_2byte, 512,		// parameters
		_4byte,			// public tags checksum
		_4byte,			// private tags checksum
		_2byte,			// flags
		1,			// player count
	_end_bs_array
};

static struct g_bs_data bs_data[] = 
{	
	// ALAN Begin: fixed compiler warnings
//	_bs_gs_login_packet, 8,		// have to manually count up lengths (sucks)
//	_bs_gs_update_packet, 104,
//	_bs_gs_query_packet, 28,
	{ _bs_gs_login_packet, 8 },
	{ _bs_gs_update_packet, 170 }, // ALAN: I believe b-net code had wrong length here..
	{ _bs_gs_query_packet, 28 }
	// ALAN End
};

static struct g_bs_data bs_gs_query_response_segment =
{
	_bs_gs_query_response_segment, 104
};

static struct g_bs_data bs_data_header =
{
	_bs_gs_header_only, 4
};

static struct g_bs_data bs_gs_query_response_header =
{
	_bs_gs_query_response_header_only, 8
};

static struct g_bs_data bs_game_data =
{
	_bs_game_data, 51
};

/* ALAN Begin: defined but not used..
static struct g_bs_data bs_game_aux_data =
{
	_bs_game_aux_data, 32
};
// ALAN End */

void byte_swap_game_data(char * p_buffer)
{
	byte_swap_data("game data", p_buffer, bs_game_data.length, 1, bs_game_data.bs_code);

	return;	
}

void byte_swap_query_response_packet(
	char *buffer, 
	boolean outgoing)
{
	char *byte_swap_me;
	int segment_index, number_of_responses= 0;
	struct gs_query_response_header *gs_header;
	
	// start byteswapping!
	// get the standard header
	byte_swap_me = (char *)buffer;
	byte_swap_data("game search packet", byte_swap_me, bs_data_header.length, 1, bs_data_header.bs_code);
	byte_swap_me += sizeof(struct room_packet_header);

	// special gs_query_response_packet_header
	if (outgoing)
	{
		gs_header= (struct gs_query_response_header *)byte_swap_me;
		number_of_responses= gs_header->number_of_responses;
	}
	byte_swap_data("game search packet", byte_swap_me, bs_gs_query_response_header.length, 1, bs_gs_query_response_header.bs_code);
	if (!outgoing)
	{
		gs_header= (struct gs_query_response_header *)byte_swap_me;
		number_of_responses= gs_header->number_of_responses;
	}

	byte_swap_me+= sizeof(struct gs_query_response_header);

	// start swapping the segments
	for (segment_index= 0; segment_index<number_of_responses; segment_index++)
	{
		int segment_length= 0;
		if (outgoing) segment_length= ((struct gs_query_response_segment *)(byte_swap_me))->segment_length;

		byte_swap_data("game search packet", byte_swap_me, 
			bs_gs_query_response_segment.length, 1, bs_gs_query_response_segment.bs_code);
		if (!outgoing) segment_length= ((struct gs_query_response_segment *)(byte_swap_me))->segment_length;
		byte_swap_me+= segment_length;
	}
}

boolean byte_swap_game_search_packet(
	char *buffer, 
	boolean outgoing)
{
	short type, length;
	struct room_packet_header *header;
	boolean success;

	header= (struct room_packet_header *)buffer;
	type= header->type;
	length= header->length;
	if (!outgoing)
	{
		type= SWAP2(type);
		length= SWAP2(length);
	}

	if (type==_gs_query_response_packet)
	{
		byte_swap_query_response_packet(buffer, outgoing);
	}
	else
	{
		byte_swap_data("game search packet", buffer, bs_data[type].length, 1, bs_data[type].bs_code);
	}

	switch (type)
	{
	case _gs_login_packet:
	case _gs_update_packet:
	case _gs_query_packet:
	case _gs_query_response_packet:
		success = TRUE;
		break;
	default:
		success = FALSE;
		break;
	}

	return success;
}

static short build_empty_header(
	char *buffer, 
	short type)
{
	struct room_packet_header *header= (struct room_packet_header *)buffer;

	/* Fill in the header */
	header->type= type;
	header->length= sizeof(struct room_packet_header);

	return header->length;
}

static short append_data_to_packet(
	char *buffer, 
	void *data, 
	short data_length)
{
	struct room_packet_header *header= (struct room_packet_header *)buffer;
	char *dest = (buffer + header->length);

	/* Fill in the header */	
	memcpy(dest, data, data_length);
	header->length+= data_length;

	return header->length;
}

static short build_empty_gs_query_response_packet_header(
	char *buffer,
	unsigned long user_id)
{
	struct room_packet_header *header = (struct room_packet_header *)buffer;
	int n = 0;
	
	header->type = _gs_query_response_packet;
	header->length = sizeof(struct room_packet_header);

	append_data_to_packet(buffer, (void *)&n, sizeof(long));
	append_data_to_packet(buffer, (void *)&user_id, sizeof(unsigned long));

	return header->length;
}

static short append_segment_to_gs_query_response_packet(
	char *buffer, 
	struct gs_query_response_segment *seg)
{
	struct room_packet_header *header = (struct room_packet_header *)buffer;
	char *dest = buffer + sizeof(struct room_packet_header);
	struct gs_query_response_header *gs_response_header = (struct gs_query_response_header *)dest;
	char scratch[2048];
	int pad;
	
	dest = (buffer + header->length);
	// each header must start on a 4 byte boundary
	// this is accomplished by padding the end of the segment with NULLS
	
	memcpy(scratch, seg, seg->segment_length);
	seg = (struct gs_query_response_segment *)scratch;

	// ALAN Begin: compiler warning fix
//	if (pad = (seg->segment_length % 4))
	if ((pad = (seg->segment_length % 4)))
	// ALAN End
	{
		int index;
		for (index = 0; index < (4 - pad); index++)
		{
			seg->segment_length++;
			scratch[seg->segment_length] = 0;
		}
	}
	
	// append segment to end of mojo response packet
	memcpy(dest, scratch, seg->segment_length);

	gs_response_header->number_of_responses++;

	header->length+= seg->segment_length;
	return header->length;
}
