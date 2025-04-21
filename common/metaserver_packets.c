/*
	metaserver_packets.c
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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "byte_swapping.h"

#define INCLUDE_DATA
#include "metaserver_common_structs.h"
#include "authentication.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "metaserver_codes.h"
#include "metaserver_common_structs.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"

/* --------- local prototypes */
static short build_empty_header(char *buffer, short type);
static short append_data_to_packet(char *buffer, void *data, short data_length);

/* ------------ server packets */
short start_building_room_packet(
	char * buffer)
{
	return build_empty_header(buffer, _room_list_packet);
}

short add_room_data_to_packet(
	char * buffer, 
	struct room_info *room)
{
	return append_data_to_packet(buffer, (void *) room, sizeof(struct room_info));
}

short build_player_info_query_packet(
	char * buffer,
	long player_id)
{
	build_empty_header(buffer, _player_info_query_packet);
	return append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
}

short build_order_query_packet(
	char * buffer,
	short order)
{
	// ALAN Begin: unused variable
//	short unused;
	// ALAN End

	return build_empty_header(buffer, _order_query_packet);
}

short build_buddy_query_packet(
	char * buffer)
{
	return build_empty_header(buffer, _buddy_query_packet);
}

short start_building_order_list_packet(
	char * buffer)
{
	return build_empty_header(buffer, _order_list_packet);
}

short start_building_buddy_list_packet(
	char * buffer)
{
	return build_empty_header(buffer, _buddy_list_packet);
}

short start_building_player_search_list_packet(
	char * buffer)
{
	return build_empty_header(buffer, _player_search_list_packet);
}

short start_building_player_list_packet(
	char * buffer)
{
	return build_empty_header(buffer, _player_list_packet);
}

short add_player_data_to_search_packet(
	char * buffer, 
	long room_id,
	struct metaserver_player_aux_data * player_aux_data,
	void * player_data)
{
	append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
	append_data_to_packet(buffer, player_aux_data, sizeof(struct metaserver_player_aux_data));
	return append_data_to_packet(buffer, player_data, player_aux_data->player_data_length);
}

short add_player_data_to_buddy_list_packet(
	char * buffer, 
	long room_id,
	struct metaserver_player_aux_data *player_aux_data,
	void * player_data)
{
	append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
	append_data_to_packet(buffer, player_aux_data, sizeof(struct metaserver_player_aux_data));
	return append_data_to_packet(buffer, player_data, player_aux_data->player_data_length);
}

short add_player_data_to_order_list_packet(
	char * buffer, 
	long room_id,
	struct metaserver_player_aux_data * player_aux_data,
	void * player_data)
{
	append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
	append_data_to_packet(buffer, player_aux_data, sizeof(struct metaserver_player_aux_data));
	return append_data_to_packet(buffer, player_data, player_aux_data->player_data_length);
}

short add_player_data_to_packet(
	char * buffer, 
	struct metaserver_player_aux_data * player_aux_data,
	void * player_data, 
	short length)
{
	player_aux_data->player_data_length= length;
	append_data_to_packet(buffer, player_aux_data, sizeof(struct metaserver_player_aux_data));
	return append_data_to_packet(buffer, player_data, player_aux_data->player_data_length);
}

short start_building_game_list_pref_packet(char * buffer)
{
	return build_empty_header(buffer, _game_list_pref_packet);
}

short start_building_game_list_packet(
	char *buffer)
{
	return build_empty_header(buffer, _game_list_packet);
}

short add_game_data_to_packet(
	char *buffer,	
	struct metaserver_game_aux_data *aux_data,
	void *game_data, 
	short length)
{
	aux_data->game_data_size= length;
	append_data_to_packet(buffer, aux_data, sizeof(struct metaserver_game_aux_data));
	return append_data_to_packet(buffer, game_data, length);
}

short build_player_info_packet(
	char * buffer, 
	boolean administrator_flag, 
	boolean bungie_employee_flag, 
	short order_index, 
	short icon_index, 
	struct rgb_color * primary_color, 
	struct rgb_color * secondary_color, 
	struct bungie_net_player_score_datum * unranked_score_datum, 
	struct bungie_net_player_score_datum * ranked_score_datum, 
	struct bungie_net_player_score_datum * ranked_score_datum_by_game_type,
	struct bungie_net_player_score_datum * order_unranked_score_datum, 
	struct bungie_net_player_score_datum * order_ranked_score_datum, 
	struct bungie_net_player_score_datum * order_ranked_score_datum_by_game_type, 
	struct overall_ranking_data * overall_rank_data, 
	char * strings)
{
	char * p;
	short n;

	build_empty_header(buffer, _player_info_packet);
	append_data_to_packet(buffer, (void *)&administrator_flag, sizeof(boolean));
	append_data_to_packet(buffer, (void *)&bungie_employee_flag, sizeof(boolean));
	append_data_to_packet(buffer, (void *)&order_index, sizeof(short));
	append_data_to_packet(buffer, (void *)&icon_index, sizeof(short));
	append_data_to_packet(buffer, (void *)primary_color, sizeof(struct rgb_color));
	append_data_to_packet(buffer, (void *)secondary_color, sizeof(struct rgb_color));

	append_data_to_packet(buffer, (void *)unranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)ranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)ranked_score_datum_by_game_type, sizeof(struct bungie_net_player_score_datum) * MAXIMUM_NUMBER_OF_GAME_TYPES);

	append_data_to_packet(buffer, (void *)order_unranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)order_ranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)order_ranked_score_datum_by_game_type, sizeof(struct bungie_net_player_score_datum) * MAXIMUM_NUMBER_OF_GAME_TYPES);

	append_data_to_packet(buffer, (void *)overall_rank_data, sizeof(struct overall_ranking_data));

	// login
	p = strings;
	n = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, n);

	// name
	p += n;
	n = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, n);

	// order name
	p += n;
	n = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, n);

	// personal data
	p += n;
	n = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, n);

	p += n;
	n = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, n);

	p += n;
	n = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, n);

	p += n;
	n = strlen(p) + 1;
	return append_data_to_packet(buffer, (void *)p, n);
}

/* Build the server message packet */
short build_server_message_packet(
	char * buffer, 
	short error_code)
{
	long long_error_code = error_code;

	assert(NUMBER_OF_MESSAGES == NUMBER_OF_MESSAGE_TYPES);
	assert(error_code >= 0 && error_code < NUMBER_OF_MESSAGE_TYPES);

	// long to preserve padding to struct alignment
	build_empty_header(buffer, _server_message_packet);
	append_data_to_packet(buffer, &long_error_code, sizeof(long_error_code));

	// +1 for the NULL
	return append_data_to_packet(buffer, messages[error_code], strlen(messages[error_code])+1);
}

short build_url_packet(
	char *buffer, 
	short error_code, 
	char *message, 
	char *url)
{
	long long_error_code = error_code;

	// long to preserve padding to struct alignment
	build_empty_header(buffer, _url_packet);
	append_data_to_packet(buffer, &long_error_code, sizeof(long_error_code));
	append_data_to_packet(buffer, message, strlen(message)+1);

	// +1 for the NULL
	return append_data_to_packet(buffer, url, strlen(url)+1);
}

short build_update_info_packet(
	char * buffer,
	long size,
	long host,
	short port)
{
	build_empty_header(buffer, _update_info_packet);
	append_data_to_packet(buffer, (void *)&size, sizeof(size));
	append_data_to_packet(buffer, (void *)&host, sizeof(host));
	append_data_to_packet(buffer, (void *)&port, sizeof(port));
	return append_data_to_packet(buffer, (void *)&port, sizeof(port));
}

short build_room_login_successful_packet(
	char *buffer, 
	unsigned long user_id,
	short maximum_players_per_room)
{
	short unused= 0;

	build_empty_header(buffer, _room_login_successful_packet);

	append_data_to_packet(buffer, &user_id, sizeof(unsigned long));
	append_data_to_packet(buffer, &maximum_players_per_room, sizeof(short));
	
	return append_data_to_packet(buffer, &unused, sizeof(short));
}

short build_data_chunk_packet(
	char *buffer, 
	struct data_chunk_identifier_data *id, 
	void *data)
{
	build_empty_header(buffer, _data_chunk_packet);
	append_data_to_packet(buffer, id, sizeof(struct data_chunk_identifier_data));
	return append_data_to_packet(buffer, data, id->length);
}

short build_password_challenge_packet(
	char *buffer, 
	short authentication_type,
	unsigned char *salt)
{
	build_empty_header(buffer, _password_challenge_packet);
	append_data_to_packet(buffer, &authentication_type, sizeof(short));
	return append_data_to_packet(buffer, salt, MAXIMUM_SALT_SIZE*sizeof(char));
}

short build_user_successful_login_packet(
	char *buffer, 
	long user_id,
	short order,
	authentication_token *token)
{
	build_empty_header(buffer, _user_successful_login_packet);
	append_data_to_packet(buffer, &user_id, sizeof(long));
	append_data_to_packet(buffer, &order, sizeof(order));
	append_data_to_packet(buffer, &order, sizeof(order));			// unused for now
	return append_data_to_packet(buffer, token, sizeof(authentication_token));
}

short build_set_player_data_from_metaserver_packet(
	char *buffer, 
	struct metaserver_player_aux_data *player, 
	char *data, 
	short data_length)
{
	build_empty_header(buffer, _set_player_data_from_metaserver_packet);
	append_data_to_packet(buffer, player, sizeof(struct metaserver_player_aux_data));
	return append_data_to_packet(buffer, data, data_length);
}

short build_message_of_the_data_packet(
	char *buffer, 
	char *message)
{
	build_empty_header(buffer, _message_of_the_day_packet);
	return append_data_to_packet(buffer, message, (short) (strlen(message)+1));
}

short build_you_just_got_blammed_sucka_packet(
        char *buffer,
        char *message)
{
        build_empty_header(buffer, _you_just_got_blammed_sucka_packet);
        return append_data_to_packet(buffer, message, (short) (strlen(message)+1));
}

short build_send_versions_packet(
	char *buffer)
{
	return build_empty_header(buffer, _send_versions_packet);
}

short build_update_player_buddy_list_packet(
	char * buffer,
	struct buddy_entry * buddies)
{
	build_empty_header(buffer, _update_player_buddy_list_packet);
	return append_data_to_packet(buffer, (void *)buddies, 
		sizeof(struct buddy_entry) * MAXIMUM_BUDDIES);
}

short build_order_member_list_packet(
	char * buffer,
	long member_count,
	struct order_member * members)
{
	long index;

	build_empty_header(buffer, _update_order_member_list_packet);
	append_data_to_packet(buffer, (void *)&member_count, sizeof(member_count));
	for (index = 0; index < member_count; index++)
	{
		append_data_to_packet(buffer, (void *)&members[index], sizeof(struct order_member));
	}

	return ((struct packet_header *)(buffer))->length;
}

/* -------- client packets */
// _MYTHDEV Begin
short build_sessionkey_packet(  
	char* buffer, 
	char* my_public_key,
	int   key_size)
{
	build_empty_header(buffer, _session_key_packet);
	return append_data_to_packet(buffer, my_public_key, key_size);
}
// _MYTHDEV End

short build_login_packet(
	char *buffer, 
	struct login_data *login)
{
	build_empty_header(buffer, _login_packet);
	return append_data_to_packet(buffer, login, sizeof(struct login_data));
}

short build_room_login_packet(
	char *buffer, 
	authentication_token token,
	char version,
	char *name)
{
	build_empty_header(buffer, _room_login_packet);
	append_data_to_packet(buffer, token, sizeof(authentication_token));
	return append_data_to_packet(buffer, name, strlen(name)+1);
}

short build_logout_packet(
	char *buffer)
{
	return build_empty_header(buffer, _logout_packet);
}

short build_set_player_data_packet(
	char *buffer, 
	char *player_data, 
	short player_data_size)
{ 
	build_empty_header(buffer, _set_player_data_packet);
	return append_data_to_packet(buffer, player_data, player_data_size);
}

short build_game_packet(
	char *buffer, 
	short game_port,
	short order_game,
	void *game_data, 
	short game_data_length)
{
	build_empty_header(buffer, _create_game_packet);
	append_data_to_packet(buffer, &game_port, sizeof(short));
	append_data_to_packet(buffer, &order_game, sizeof(short));
	return append_data_to_packet(buffer, game_data, game_data_length);
}

short build_reset_game_packet(
	char *buffer)
{
	return build_empty_header(buffer, _reset_game_packet);
}

short build_remove_game_packet(
	char *buffer)
{
	return build_empty_header(buffer, _remove_game_packet);
}

short build_change_room_packet(
	char *buffer, 
	short new_room_id)
{
	build_empty_header(buffer, _change_room_packet);
	return append_data_to_packet(buffer, &new_room_id, sizeof(short));
}

short build_set_player_mode_packet(
	char *buffer, 
	short new_mode)
{
	build_empty_header(buffer, _set_player_mode_packet);
	return append_data_to_packet(buffer, &new_mode, sizeof(short));
}

short build_data_chunk_reply_packet(
	char *buffer, 
	struct data_chunk_identifier_data *id)
{
	build_empty_header(buffer, _data_chunk_reply_packet);
	return append_data_to_packet(buffer, id, sizeof(struct data_chunk_identifier_data));
}

short build_password_response_packet(
	char *buffer, 
	char *encrypted_password)
{
	build_empty_header(buffer, _password_response_packet);
	return append_data_to_packet(buffer, encrypted_password, strlen(encrypted_password)+1);
}

short build_request_full_update_packet(
	char *buffer)
{
	return build_empty_header(buffer, _request_full_update_packet);
}

short build_start_game_packet(
	char *buffer, 
	long game_time_in_seconds)
{
	long unused;

	build_empty_header(buffer, _start_game_packet);
	append_data_to_packet(buffer, &game_time_in_seconds, sizeof(long));
	append_data_to_packet(buffer, &unused, sizeof(long));
	return append_data_to_packet(buffer, &unused, sizeof(long));
}

short start_building_game_player_list_packet(
	char *buffer,
	long game_id)
{
	build_empty_header(buffer, _game_player_list_packet);
	return append_data_to_packet(buffer, &game_id, sizeof(long));
}

short add_game_player_data_to_packet(
	char *buffer, 
	struct player_list_packet_entry *entry)
{
	return append_data_to_packet(buffer, entry, sizeof(struct player_list_packet_entry));
}

short build_game_score_packet(
	char * buffer, 
	unsigned long game_id, 
	struct bungie_net_game_standings * standings)
{
	build_empty_header(buffer, _game_score_packet);
	append_data_to_packet(buffer, (void *)&game_id, sizeof(game_id));
	return append_data_to_packet(buffer, (void *)standings, sizeof(struct bungie_net_game_standings));
}

short add_game_player_score_data_to_packet(
	char *buffer,
	struct score_list_packet_entry *entry)
{
	return append_data_to_packet(buffer, entry, sizeof(struct score_list_packet_entry));
}

short build_update_buddy_packet(
	char * buffer, 
	long buddy_id,
	boolean add)
{
	short length;

	length = build_empty_header(buffer, _update_buddy_packet);
	append_data_to_packet(buffer, (void *)&buddy_id, sizeof(buddy_id));
	return append_data_to_packet(buffer, (void *)&add, sizeof(add));
}

short build_version_control_packet(
	char *buffer, 
	long platform_type,
	long application_version,
	long patch_number,
	long bnupdate_version)
{
	build_empty_header(buffer, _version_control_packet);
	append_data_to_packet(buffer, (void *)&platform_type, sizeof(platform_type));
	append_data_to_packet(buffer, (void *)&application_version, sizeof(application_version));
	append_data_to_packet(buffer, (void *)&patch_number, sizeof(patch_number));
	return append_data_to_packet(buffer, (void *)&bnupdate_version, sizeof(bnupdate_version));
}

short build_player_search_query_packet(
	char * buffer, 
	char * string)
{
	build_empty_header(buffer, _player_search_query_packet);
	return append_data_to_packet(buffer, (void *)string, strlen(string) + 1);
}

short build_game_search_query_packet(
	char *buffer, 
	struct game_search_query *query)
{
	build_empty_header(buffer, _game_search_query_packet);
	append_data_to_packet(buffer, (void *)&query->game_type, sizeof(query->game_type));
	append_data_to_packet(buffer, (void *)&query->game_scoring, sizeof(query->game_scoring));
	append_data_to_packet(buffer, (void *)&query->veterans, sizeof(query->veterans));
	append_data_to_packet(buffer, (void *)&query->unit_trading, sizeof(query->unit_trading));
	append_data_to_packet(buffer, (void *)&query->teams, sizeof(query->teams));
	append_data_to_packet(buffer, (void *)&query->alliances, sizeof(query->alliances));
	append_data_to_packet(buffer, (void *)&query->enemy_visibility, sizeof(query->enemy_visibility));
	append_data_to_packet(buffer, (void *)&query->enemy_visibility, sizeof(query->enemy_visibility));
	append_data_to_packet(buffer, (void *)query->game_name, strlen(query->game_name) + 1);
	return append_data_to_packet(buffer, (void *)query->map_name, strlen(query->map_name) + 1);
}

short build_update_player_information_packet(
	char * buffer, 
	char * city,
	char * state,
	char * country,
	char * personal)
{
	build_empty_header(buffer, _update_player_information_packet);
	append_data_to_packet(buffer, (void *)city, strlen(city) + 1);
	append_data_to_packet(buffer, (void *)state, strlen(state) + 1);
	append_data_to_packet(buffer, (void *)country, strlen(country) + 1);
	return append_data_to_packet(buffer, (void *)personal, strlen(personal) + 1);
}

/* ------- packets sent by both */
short build_room_broadcast_packet(
	char *buffer, 
	char *broadcast_data, 
	short broadcast_data_length)
{
	build_empty_header(buffer, _room_broadcast_packet);
	return append_data_to_packet(buffer, broadcast_data, broadcast_data_length);
}

short build_directed_data_packet(
	char *buffer, 
	unsigned long player_id,
	boolean echo,
	char *data, 
	short data_length)
{
	build_empty_header(buffer, _directed_data_packet);
	append_data_to_packet(buffer, &player_id, sizeof(unsigned long));
	append_data_to_packet(buffer, &echo, sizeof(echo));
	return append_data_to_packet(buffer, data, data_length);
}

short build_keepalive_packet(
	char *buffer)
{
	return build_empty_header(buffer, _keepalive_packet);
}

/* ----------- parsing code */
// this needs to be cleaned up (like the network_queues stuff)
boolean parse_network_stream(
	struct circular_buffer *buffer, 
	struct packet_header *packet)
{
	boolean got_a_packet= FALSE;

	if(CIRCULAR_BUFFER_WRITTEN_SIZE(buffer)>=sizeof(struct packet_header))
	{
		short initial_read_index= buffer->read_index;
		byte *dest= (byte *) packet;
		long length;
	
		// read the header (it could straddle)
		length= sizeof(struct packet_header);
		while(length-->0)
		{
			*dest++= buffer->buffer[initial_read_index];
			if(++initial_read_index>=buffer->size) initial_read_index= 0;
		}
		
		length= packet->length;
#ifdef little_endian
		length= SWAP4(packet->length);
#endif

		if(CIRCULAR_BUFFER_WRITTEN_SIZE(buffer)>=length)
		{
			char *dest;
			short index;
		
			/* Okay, this is totally valid- copy into the linear packet */
			dest= (char *) packet;
			for(index= 0; index<length; ++index)
			{
				*dest++= buffer->buffer[buffer->read_index];
				INCREMENT_CIRCULAR_BUFFER_READ_INDEX(buffer);
			}

			/* And get out.. */
			got_a_packet= TRUE;
		}
	}

	return got_a_packet;
}

static short build_empty_header(
	char *buffer,
	short type)
{
	struct packet_header *header= (struct packet_header *) buffer;

	/* Fill in the header */
	header->packet_identifier= PACKET_IDENTIFIER;
	header->type= type;
	header->length= sizeof(struct packet_header);

	return header->length;
}

static short append_data_to_packet(
	char *buffer,
	void *data,
	short data_length)
{
	struct packet_header *header= (struct packet_header *) buffer;
	char *dest= (buffer+header->length);

	assert(header->packet_identifier==PACKET_IDENTIFIER);

	/* Fill in the header */	
	memcpy(dest, data, data_length);
	header->length+= data_length;

	return header->length;
}


#ifdef little_endian
boolean byteswap_packet(
	char *buffer,
	boolean outgoing)
{
	short type;
	long packet_length;
	struct packet_header *header= (struct packet_header *) buffer;
	boolean success= TRUE;
	byte_swap_code _bs_header[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
		_end_bs_array };
	byte_swap_code _bs_header_followed_by_long[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, // error_code
		_end_bs_array };
	byte_swap_code _bs_user_successful_login_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, _2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_room_login_successful_packet[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, // user_id
			_2byte, // maximum_players_per_room
			_2byte, // unused short
		_end_bs_array };
	byte_swap_code _bs_header_followed_by_short[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_2byte, // error_code
		_end_bs_array };
	byte_swap_code _bs_long[] = {
		_begin_bs_array, 1,
			_4byte,
		_end_bs_array };
	byte_swap_code _bs_metaserver_player_aux_data[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, _4byte, _4byte, _2byte, _2byte, _2byte, _2byte, // verb, flags, player_id, ranking, room_id, caste, padding
		_end_bs_array };
	byte_swap_code _bs_data_chunk_identifier_data[]= {
		_begin_bs_array, 1,
			_4byte, _4byte, _4byte, _4byte, // type, length, offset, flags
		_end_bs_array };
	byte_swap_code _bs_player_list_entry[]= {
		_begin_bs_array, 1,
			_4byte,
		_end_bs_array };
	byte_swap_code _bs_room_info_data[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, // room_id, player_count
			_4byte, // host
			_2byte, _2byte, // port, game_count
			_2byte, _2byte,	// room_type, unused
			_2byte, _2byte,	// unused
			_2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_login_packet[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_2byte, _2byte, // metaserver_version, platform_type, 
			_4byte, _4byte, // flags, user_id
			_2byte, _2byte, // max_authentication, player_data_size,
			MAXIMUM_METASERVER_APPLICATION_NAME+1,
			MAXIMUM_METASERVER_BUILD_DATE+1,
			MAXIMUM_METASERVER_BUILD_TIME+1,
			MAXIMUM_METASERVER_USERNAME+1,
			MAXIMUM_METASERVER_PLAYER_DATA_SIZE,
			_2byte, // country code.
			_2byte, // padding
		_end_bs_array };
	byte_swap_code _bs_directed_data_packet[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, _4byte,// error_code
		_end_bs_array };

	// ALAN Begin: unused 'variable'
/*	byte_swap_code _bs_score_list_packet_entry[]= {
		_begin_bs_array, 1,
			_4byte, // player_id
			_2byte, // place
			_2byte, // kills
			_2byte, // casualties
			_2byte, // points_killed
			_2byte, // points_lost,
			3*sizeof(short), // unused
		_end_bs_array };
*/
	// ALAN End

	byte_swap_code _bs_metaserver_game_aux_data[]= {
		_begin_bs_array, 1,
			_4byte, _4byte, // game_id, host
			_2byte, sizeof(char), sizeof(char), // port, verb, version
			_4byte, // game_ticks remaining
			_4byte, // player id of game creator
			_2byte, sizeof(short), // game_data_size, unused_short
			2*sizeof(long), // unused longs
		_end_bs_array };
	byte_swap_code _bs_set_player_data_from_metaserver_packet[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_2byte, _2byte,			// player_aux_data
			_4byte, _4byte, _4byte,
			_2byte, _2byte, 
			_2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_player_info_packet_prefix[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, _4byte, _2byte, _2byte,
			_2byte, _2byte, _2byte, _2byte,
			_2byte, _2byte, _2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_bungie_net_player_score_datum[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _2byte, _2byte,			// games played, wins, losses, ties
			_4byte, _4byte,							// damage inflicted / damage received
			_2byte, _2byte, _2byte, _2byte,			// disconnects, pad, points, rank
			_2byte, _2byte,							// highest points / highest rank
			_4byte, 16,
		_end_bs_array };
	byte_swap_code _bs_start_game_packet[]= {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, // game time in seconds
			2*sizeof(long), // unused
		_end_bs_array };
	byte_swap_code _bs_create_game_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_game_search_query_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_2byte, _2byte, _2byte, _2byte,
			_2byte, _2byte, _2byte, _2byte, 
		_end_bs_array };
	byte_swap_code _bs_update_buddy_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, _4byte,			// buddy_id, add
		_end_bs_array };
	byte_swap_code _bs_version_control_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, _4byte, _4byte,	// platform application version, patch number
			_4byte,					// bnupdate_version
		_end_bs_array };
	byte_swap_code _bs_update_info_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte, // header
			_4byte, _4byte, _2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_update_player_buddy_list_packet[] = {
		_begin_bs_array, 1, 
			_2byte, _2byte, _4byte,
			_4byte, 4, _4byte, 4,
			_4byte, 4, _4byte, 4,
			_4byte, 4, _4byte, 4,
			_4byte, 4, _4byte, 4,
			// _MYTHDEV Begin: byteswap 8 extra buddies  :)
			_4byte, 4, _4byte, 4,
			_4byte, 4, _4byte, 4,
			_4byte, 4, _4byte, 4,
			_4byte, 4, _4byte, 4,
			// _MYTHDEV End
		_end_bs_array };
	byte_swap_code _bs_update_order_member_list_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte,
			_4byte,
		_end_bs_array };
	byte_swap_code _bs_order_member[] = {
		_begin_bs_array, 1,
			_4byte, _4byte,
		_end_bs_array };
	byte_swap_code _bs_game_score_packet[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _4byte,
			_4byte,
			_2byte, _2byte, _4byte,
			_4byte, _4byte,
			_4byte, _2byte, _2byte,
		_end_bs_array };
	byte_swap_code _bs_bungie_net_game_standings_team[] = {
		_begin_bs_array, 1,
			_2byte, _2byte, _2byte, _2byte,
			_4byte,
		_end_bs_array };
	byte_swap_code _bs_bungie_net_game_standings_player[] = {
		_begin_bs_array, 1, 
			_2byte, _2byte, _4byte,
			_4byte, _4byte, 
			_4byte, _4byte, 
			_4byte,
		_end_bs_array };
	byte_swap_code _bs_game_rank_data[] = {
		_begin_bs_array, 1,
			MAXIMUM_PLAYER_NAME_LENGTH + 1,
			_4byte, _4byte,
			_4byte, _4byte,
			_4byte, _4byte, 
			_4byte, _4byte,
			_4byte, _4byte,
		_end_bs_array };

	type= header->type;
	packet_length= header->length;
	if(!outgoing) 
	{
		type= SWAP2(type);
		packet_length= SWAP4(packet_length);
	}

	switch(type)
	{
		case _game_score_packet:
			{
				short index;

				char * p = buffer;

				byte_swap_data("game score packet", buffer,
					sizeof(struct packet_header) + sizeof(struct game_score_packet) -
					sizeof(struct bungie_net_game_standings_team) * MAXIMUM_TEAMS_PER_MAP -
					sizeof(struct bungie_net_game_standings_player) * MAXIMUM_PLAYERS_PER_MAP, 1,
					_bs_game_score_packet);

				p += sizeof(struct packet_header) + sizeof(struct game_score_packet) -
					sizeof(struct bungie_net_game_standings_team) * MAXIMUM_TEAMS_PER_MAP -
					sizeof(struct bungie_net_game_standings_player) * MAXIMUM_PLAYERS_PER_MAP;

				for (index = 0; index < MAXIMUM_TEAMS_PER_MAP; ++index)
				{
					byte_swap_data("game score packet", p,
						sizeof(struct bungie_net_game_standings_team), 1, 
						_bs_bungie_net_game_standings_team);
					p += sizeof(struct bungie_net_game_standings_team);
				}

				for (index = 0; index < MAXIMUM_PLAYERS_PER_MAP; ++index)
				{
					byte_swap_data("game score packet", p,
						sizeof(struct bungie_net_game_standings_player), 1, 
						_bs_bungie_net_game_standings_player);
					p += sizeof(struct bungie_net_game_standings_player);
				}
			}
			break;

		case _update_order_member_list_packet:
			{
				short member_count= 0;
				long index;
				char * p;

				p = buffer;
				p += sizeof(struct packet_header);

				if (outgoing)
				{
					member_count = ((struct update_order_member_list_packet *)(p))->member_count;
				}

				byte_swap_data("order member packet", buffer,
					sizeof(struct packet_header) +
					sizeof(struct update_order_member_list_packet), 1,
					_bs_update_order_member_list_packet);

				if (!outgoing)
				{
					member_count = ((struct update_order_member_list_packet *)(p))->member_count;
				}

				p += sizeof(struct update_order_member_list_packet);
				for (index = 0; index < member_count; index++)
				{
					byte_swap_data("order member", p,
						sizeof(struct order_member), 1,
						_bs_order_member);
					p += sizeof(struct order_member);
				}
			}
			break;

		case _update_player_buddy_list_packet:
			byte_swap_data("version control packet", buffer, 
				sizeof(struct packet_header) +
				sizeof(struct update_player_buddy_list_packet), 1,
				_bs_update_player_buddy_list_packet);
			break;

		case _room_list_packet:
		case _game_player_list_packet:
			{
				short dynamic_entry_size= 0, header_size= 0;
				byte_swap_code *byteswap_codes= 0;
				byte_swap_code *header_byteswap_codes= 0;
				short index, elem_count;
				long offset;
				
				switch(type)
				{
					case _room_list_packet:
						dynamic_entry_size= sizeof(struct room_info);
						byteswap_codes= _bs_room_info_data;
						header_size= sizeof(struct packet_header);
						header_byteswap_codes= _bs_header;
						break;

					case _game_player_list_packet:
						dynamic_entry_size= sizeof(struct player_list_packet_entry);
						byteswap_codes= _bs_player_list_entry;
						header_size= sizeof(struct packet_header)+sizeof(long);
						header_byteswap_codes= _bs_header_followed_by_long;
						break;

					default: 
						halt();
						break;
				}

				byte_swap_data("entry header", buffer, header_size, 1, header_byteswap_codes);

				elem_count = (packet_length-header_size)/dynamic_entry_size;

				if ((header_size + (elem_count * dynamic_entry_size)) != packet_length)
				{
					return FALSE;
				}
				// end
	
				for(index= 0, offset= header_size; index<elem_count; ++index, offset+= dynamic_entry_size)
				{
					char *entry= ((char *) header+offset);

					byte_swap_data("dynamic list entry", entry, dynamic_entry_size, 1, byteswap_codes);
				}
			}
			break;

		case _order_list_packet:
		case _buddy_list_packet:
		case _player_search_list_packet:
			{
				short data_size = sizeof(struct packet_header);
				long length = packet_length;
				char * byte_swap_me = buffer;
				byte_swap_code * byteswap_codes;

				byteswap_codes = _bs_header;
				byte_swap_data("entry header", byte_swap_me, data_size, 1, byteswap_codes);

				// check to see if there were no replies
				if (length == sizeof(struct packet_header))
					break;

				length -= data_size;
				byte_swap_me += data_size;
				while (length > 0)
				{
					short offset= 0;

					byteswap_codes = _bs_long;
					data_size = sizeof(long);
					byte_swap_data("room id", byte_swap_me, data_size, 1, byteswap_codes);
					byte_swap_me += data_size;					
					length -= data_size;
					byteswap_codes = _bs_metaserver_player_aux_data;
					data_size = sizeof(struct metaserver_player_aux_data);
					if (outgoing)
						offset = data_size + ((struct metaserver_player_aux_data *)byte_swap_me)->player_data_length;
					byte_swap_data("entry header", byte_swap_me, data_size, 1, byteswap_codes);
					if (!outgoing)
						offset = data_size + ((struct metaserver_player_aux_data *)byte_swap_me)->player_data_length;
					length -= offset;
					byte_swap_me += offset;
				}
			}
			break;

		case _player_list_packet:
			// swap all the hosts...
			{
				long length= packet_length;
				
				byte_swap_data("player list packet header", buffer, sizeof(struct packet_header), 1, _bs_header);
				length-= sizeof(struct packet_header);
				while(length>0)
				{
					struct metaserver_player_aux_data *aux_data;
					short player_data_size;
					long offset= packet_length-length;

					aux_data= (struct metaserver_player_aux_data *) ((char *) buffer+offset);
					if(outgoing)
					{
						player_data_size= aux_data->player_data_length;
					} else {
						player_data_size= SWAP2(aux_data->player_data_length);
					}
					byte_swap_data("player list packet entry", aux_data, 
						sizeof(struct metaserver_player_aux_data), 1, _bs_metaserver_player_aux_data);

					// increment lengths and offsets, sos we are the right place next time.
					length-= (sizeof(struct metaserver_player_aux_data)+player_data_size);
				}
				if (length != 0)
				{
					return FALSE;
				}
			}
			break;
			
		case _server_message_packet:
		case _url_packet:
		case _player_info_query_packet:
			byte_swap_data("sizeof(struct packet header)+sizeof(long)", buffer, 
				sizeof(struct packet_header)+sizeof(long), 
				1, _bs_header_followed_by_long);
			break;

		case _update_info_packet:
			byte_swap_data("update info packet", buffer, 
				sizeof(struct packet_header) + sizeof(struct update_info_packet), 
				1, _bs_update_info_packet);
			break;

		case _version_control_packet:
			byte_swap_data("version control packet", buffer, 
				sizeof(struct packet_header) + sizeof(struct version_control_packet), 
				1, _bs_version_control_packet);
			break;

		case _user_successful_login_packet:
			byte_swap_data("user login successful", buffer, 
				sizeof(struct packet_header) + sizeof(struct user_successful_login_packet), 
				1, _bs_user_successful_login_packet);
			break;

		case _player_info_packet:
			{
				short n;
				char * p;

				p = buffer;

				byte_swap_data("player info", p, 
					36,
					1, _bs_player_info_packet_prefix);

				p += 36;
				
				for (n = 0; n < NUMBER_OF_SCORING_DATUMS_IN_PLAYER_INFO_PACKET; ++n)
				{
					byte_swap_data("player info", p, sizeof(struct bungie_net_player_score_datum), 1, 
						_bs_bungie_net_player_score_datum);
					p += sizeof(struct bungie_net_player_score_datum);
				}

				byte_swap_data("player info", p, sizeof(long), 1, _bs_long);
				p += sizeof(long);	
				byte_swap_data("player info", p, sizeof(long), 1, _bs_long);
				p += sizeof(long);

				for (n = 0; n < NUMBER_OF_GAME_RANK_DATAS_IN_OVERALL_RANKING_DATA; ++n)
				{
					byte_swap_data("player info", p, sizeof(struct game_rank_data), 1, 
						_bs_game_rank_data);
					p += sizeof(struct game_rank_data);
				}
			}
			break;

		case _room_login_successful_packet:
			byte_swap_data("_room_login_successful_packet", buffer, 
				sizeof(struct packet_header) + sizeof(struct room_login_successful_data), 
				1, _bs_room_login_successful_packet);
			break;
		
		case _login_packet:
			byte_swap_data("login packet", buffer, 
				sizeof(struct packet_header)+sizeof(struct login_data), 
				1, _bs_login_packet);
			break;

		case _game_search_query_packet:
			byte_swap_data("gs query packet", buffer, 
				sizeof(struct packet_header) + sizeof(struct game_search_query_packet), 
				1, _bs_game_search_query_packet);
			break;
			
		case _set_player_data_from_metaserver_packet:
			byte_swap_data("set player data from metaserver", buffer, 
				sizeof(struct packet_header) + sizeof(struct metaserver_player_aux_data), 
				1, _bs_set_player_data_from_metaserver_packet);
			break;
		
		case _game_list_packet:
		case _game_list_pref_packet:
			// swap all the hosts...
			{
				long length= packet_length;
				
				byte_swap_data("game list packet", buffer, sizeof(struct packet_header), 1, _bs_header);
				length-= sizeof(struct packet_header);
				while(length>0)
				{
					struct metaserver_game_aux_data	*aux_data;
					short game_data_size;
					long offset= packet_length-length;
					
					aux_data= (struct metaserver_game_aux_data	*) ((char *) buffer+offset);
					if(outgoing)
					{
						game_data_size= aux_data->game_data_size;
					} else {
						game_data_size= SWAP2(aux_data->game_data_size);
					}
					byte_swap_data("game list packet entry", aux_data, 
						sizeof(struct metaserver_game_aux_data), 1, _bs_metaserver_game_aux_data);

					// increment lengths and offsets, sos we are the right place next time.
					length-= (sizeof(struct metaserver_game_aux_data)+game_data_size);
				}
				if (length != 0)
				{
					return FALSE;
				}
			}
			break;

		case _data_chunk_packet:
		case _data_chunk_reply_packet:
			// swap the header
			byte_swap_data("packet header for datachunk", buffer, sizeof(struct packet_header), 1, _bs_header);
			
			// swap the struct data_chunk_identifier_data
			byte_swap_data("datachunk identifier", buffer+sizeof(struct packet_header), sizeof(struct data_chunk_identifier_data), 1, 
				_bs_data_chunk_identifier_data);
			break;

		case _update_buddy_packet:
			byte_swap_data("buddy list update", buffer, sizeof(struct packet_header) + sizeof(struct update_buddy_packet), 1,
				_bs_update_buddy_packet);
			break;

		// _MYTHDEV Begin
		case _session_key_packet:
		// _MYTHDEV End
		case _set_player_data_packet:
		case _logout_packet:
		case _room_broadcast_packet:
		case _reset_game_packet:
		case _remove_game_packet:
		case _keepalive_packet:
		case _room_login_packet:
		case _request_full_update_packet:
		case _password_response_packet:
		case _message_of_the_day_packet:
		case _send_versions_packet:
		case _player_search_query_packet:
		case _buddy_query_packet:
		case _order_query_packet:
		case _update_player_information_packet:
	        case _you_just_got_blammed_sucka_packet:
			byte_swap_data("packet header", buffer, sizeof(struct packet_header), 1, _bs_header);
			break;
		
		case _password_challenge_packet:
		case _change_room_packet:
		case _set_player_mode_packet:		
			byte_swap_data("packet_header+short", buffer, sizeof(struct packet_header)+sizeof(short), 1, _bs_header_followed_by_short);
			break;

		case _create_game_packet:
			byte_swap_data("create game packet", buffer, sizeof(struct packet_header) + sizeof(struct create_game_packet), 1, _bs_create_game_packet);
			break;
		
		case _directed_data_packet:
			byte_swap_data("directed data packet", buffer, sizeof(struct packet_header)+2*sizeof(long), 1, _bs_directed_data_packet);
			break;
			
		case _start_game_packet:
			byte_swap_data("start game packet", buffer, sizeof(struct packet_header)+sizeof(struct start_game_packet), 1, _bs_start_game_packet);
			break;
			
		default:
			success= FALSE;
			break;		
	}
	
	return success;
}
#else
boolean byteswap_packet(
	char *buffer,
	boolean outgoing)
{
	short type;
	struct packet_header *header= (struct packet_header *) buffer;
	boolean success;

	switch(header->type)
	{
		// _MYTHDEV Begin
		case _session_key_packet:
		// _MYTHDEV End
		case _room_list_packet:
		case _game_player_list_packet:
		case _version_control_packet:
		case _player_list_packet:
		case _server_message_packet:
		case _url_packet:
		case _user_successful_login_packet:
		case _room_login_successful_packet:
		case _login_packet:
		case _set_player_data_from_metaserver_packet:
		case _game_list_packet:
		case _data_chunk_packet:
		case _data_chunk_reply_packet:
		case _set_player_data_packet:
		case _logout_packet:
		case _room_broadcast_packet:
		case _reset_game_packet:
		case _remove_game_packet:
		case _keepalive_packet:
		case _room_login_packet:
		case _request_full_update_packet:
		case _password_response_packet:
		case _message_of_the_day_packet:
		case _send_versions_packet:
		case _password_challenge_packet:
		case _change_room_packet:
		case _set_player_mode_packet:
		case _create_game_packet:
		case _directed_data_packet:
		case _start_game_packet:
		case _update_buddy_packet:
		case _order_query_packet:
		case _player_info_packet:
		case _player_info_query_packet:
			success= TRUE;
			break;
			
		default:
			vpause(csprintf(temporary, "What is packet type 0x%x?", header->type));
			success= FALSE;
			break;		
	}
	
	return success;
}
#endif
