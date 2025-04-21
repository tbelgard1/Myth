/*
	room_packets.c
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

// ALAN Begin: added headers
#include <string.h>
// ALAN End

#include "cseries.h"
#include "byte_swapping.h"
#include "metaserver_common_structs.h"
#include "authentication.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "game_search_packets.h"

#include "environment.h"

/* -------- local code */
static short build_empty_header(char *buffer, short type);
static short append_data_to_packet(char *buffer, void *data, short data_length);
static short build_empty_player_query_response_header(
	char * buffer,
	unsigned long player_id);

/* -------- code */
short build_rs_login_successful_packet(
	char *buffer, 
	short identifier,
	short game_type,
	short player_data_size,
	short game_data_size,
	short ranked,
	short tournament_room,
	struct caste_breakpoint_data * caste_breakpoints,
	char *url_for_version_update,
	char *motd)
{
	build_empty_header(buffer, _rs_login_successful_packet);
	append_data_to_packet(buffer, &identifier, sizeof(short));
	append_data_to_packet(buffer, &game_type, sizeof(short));
	append_data_to_packet(buffer, &player_data_size, sizeof(short));
	append_data_to_packet(buffer, &game_data_size, sizeof(short));
	append_data_to_packet(buffer, &ranked, sizeof(short));
	append_data_to_packet(buffer, &tournament_room, sizeof(short));
	append_data_to_packet(buffer, caste_breakpoints, sizeof(struct caste_breakpoint_data));
	append_data_to_packet(buffer, url_for_version_update, strlen(url_for_version_update)+1);

	return append_data_to_packet(buffer, motd, strlen(motd)+1);
}

// accessors for server packets.
char *extract_url_for_update_from_rs_login_successful_packet(
	char *buffer)
{
	struct room_packet_header *header= (struct room_packet_header *) buffer;
	char *url;
	
	assert(header->type==_rs_login_successful_packet);
	url= buffer+sizeof(struct room_packet_header)+sizeof(struct rs_login_successful_packet);

	return url;
}

char *extract_motd_from_rs_login_successful_packet(
	char *buffer)
{
	struct room_packet_header *header= (struct room_packet_header *) buffer;
	char *url;
	char *message;
	
	assert(header->type==_rs_login_successful_packet);
	url= buffer+sizeof(struct room_packet_header)+sizeof(struct rs_login_successful_packet);
	message= url+(strlen(url)+1);

	return message;
}

short build_rs_login_failure_packet(
	char *buffer, 
	short code)
{
	build_empty_header(buffer, _rs_login_failure_packet);
	return append_data_to_packet(buffer, &code, sizeof(short));
}

short build_rs_client_ranking_packet(
	char *buffer, 
	unsigned long player_id, 
	short flags, 
	struct player_stats *stats)
{
	short padding= 0;

	build_empty_header(buffer, _rs_client_ranking_packet);
	append_data_to_packet(buffer, &player_id, sizeof(unsigned long));
	append_data_to_packet(buffer, &flags, sizeof(short));
	append_data_to_packet(buffer, &padding, sizeof(short));
	return append_data_to_packet(buffer, stats, sizeof(struct player_stats));
}

short build_rs_motd_changed_packet(
	char *buffer, 
	char *motd)
{
	build_empty_header(buffer, _rs_motd_changed_packet);

	return append_data_to_packet(buffer, motd, strlen(motd)+1);
}

short build_rs_ping_packet(
	char *buffer)
{
	return build_empty_header(buffer, _rs_ping_packet);
}

char *extract_motd_from_rs_motd_changed_packet(
	char *buffer)
{
	struct room_packet_header *header= (struct room_packet_header *) buffer;
	char *message;
	
	assert(header->type==_rs_motd_changed_packet);
	message= (char *)header+sizeof(struct room_packet_header);

	return message;
}

short build_rs_send_status_packet(
	char *buffer)
{
	return build_empty_header(buffer, _rs_send_status_packet);
}

short start_building_rs_room_packet(
	char *buffer)
{
	return build_empty_header(buffer, _rs_room_list_packet);
}

short add_room_to_rs_room_packet(
	char *buffer, 
	struct player_room_list_data *room)
{
	return append_data_to_packet(buffer, room, sizeof(struct player_room_list_data));
}

short build_rs_player_information_query_packet(
	char * buffer,
	long player_id)
{
	build_empty_header(buffer, _rs_player_information_query_packet);
	return append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
}

short build_rs_player_information_packet (
	char * buffer,
	long player_id,
	struct buddy_entry * buddies,
	short order,
	boolean player_is_admin,
	boolean player_is_bungie_employee,
        boolean account_is_kiosk,
	short country_code,
	char * login_name)
{
	build_empty_header(buffer, _rs_player_information_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)buddies, sizeof(struct buddy_entry) * MAXIMUM_BUDDIES);
	append_data_to_packet(buffer, (void *)&order, sizeof(order));
	append_data_to_packet(buffer, (void *)&country_code, sizeof(country_code));	// unused
	append_data_to_packet(buffer, (void *)&player_is_admin, sizeof(player_is_admin));
	append_data_to_packet(buffer, (void *)&player_is_bungie_employee, sizeof(player_is_bungie_employee));
        append_data_to_packet(buffer, (void *)&account_is_kiosk, sizeof(account_is_kiosk));
	return append_data_to_packet(buffer, (void *)login_name, strlen(login_name) + 1);
}

short build_rs_update_buddy_response_packet(
	char * buffer,
	long player_id,
	struct buddy_entry * buddies)
{
	build_empty_header(buffer, _rs_update_buddy_response_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)buddies, sizeof(struct buddy_entry) * MAXIMUM_BUDDIES);
}

short build_rs_update_buddy_packet(
	char * buffer,
	long player_id,
	long buddy_id,
	boolean add)
{
	build_empty_header(buffer, _rs_update_buddy_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)&buddy_id, sizeof(buddy_id));
	return append_data_to_packet(buffer, (void *)&add, sizeof(add));
}

short build_rs_update_order_status_packet(
	char * buffer, 
	unsigned long player_id,
	short member_count,
	struct order_member * order_members)
{
	short index;

	build_empty_header(buffer, _rs_update_order_status_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)&member_count, sizeof(member_count));
	append_data_to_packet(buffer, (void *)&member_count, sizeof(member_count));

	for (index = 0; index < member_count; index++)
	{
		append_data_to_packet(buffer, (void *)&order_members[index], sizeof(struct order_member));
	}

	return ((struct room_packet_header *)(buffer))->length;
}

short build_rs_player_info_reply_packet(
	char * buffer,
	unsigned long player_id,
	struct bungie_net_player_stats * stats)
{
	char * p;
	short s;

	build_empty_header(buffer, _rs_player_info_reply_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));

	append_data_to_packet(buffer, (void *)&stats->administrator_flag, sizeof(boolean));
	append_data_to_packet(buffer, (void *)&stats->bungie_employee_flag, sizeof(boolean));
	append_data_to_packet(buffer, (void *)&stats->order_index, sizeof(short));
	append_data_to_packet(buffer, (void *)&stats->icon_index, sizeof(short));
	append_data_to_packet(buffer, (void *)&stats->primary_color, sizeof(struct rgb_color));
	append_data_to_packet(buffer, (void *)&stats->secondary_color, sizeof(struct rgb_color));
	append_data_to_packet(buffer, (void *)&stats->unranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)&stats->ranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)&stats->ranked_score_datum_by_game_type[0], 
		sizeof(struct bungie_net_player_score_datum) * MAXIMUM_NUMBER_OF_GAME_TYPES);
	append_data_to_packet(buffer, (void *)&stats->order_unranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)&stats->order_ranked_score_datum, sizeof(struct bungie_net_player_score_datum));
	append_data_to_packet(buffer, (void *)&stats->order_ranked_score_datum_by_game_type[0], 
		sizeof(struct bungie_net_player_score_datum) * MAXIMUM_NUMBER_OF_GAME_TYPES);

	append_data_to_packet(buffer, (void *)stats->login, strlen(stats->login) + 1);
	append_data_to_packet(buffer, (void *)stats->name, strlen(stats->name) + 1);
	append_data_to_packet(buffer, (void *)stats->order_name, strlen(stats->order_name) + 1);

	p = stats->description;
	s = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, s);

	p += s;
	s = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, s);

	p += s;
	s = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, s);

	p += s;
	s = strlen(p) + 1;
	return append_data_to_packet(buffer, (void *)p, s);
}

short build_rs_global_message_packet(
	char * buffer, 
	unsigned long player_id,
	char * message)
{
	build_empty_header(buffer, _rs_global_message_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)message, strlen(message) + 1);
}

short build_rs_rank_update_packet(
	char * buffer,
	struct caste_breakpoint_data * caste_breakpoints,
	struct overall_ranking_data * overall_rank_data)
{
	build_empty_header(buffer, _rs_rank_update_packet);
	append_data_to_packet(buffer, (void *)caste_breakpoints, sizeof(struct caste_breakpoint_data));
	return append_data_to_packet(buffer, (void *)overall_rank_data, 
		sizeof(struct overall_ranking_data));
}

/* ------ client packets */
short build_rs_login_packet(
	char *buffer, 
	short port,
	short identifier,
	char *password)
{
	build_empty_header(buffer, _rs_login_packet);
	append_data_to_packet(buffer, &port, sizeof(short));
	append_data_to_packet(buffer, &identifier, sizeof(short));
	return append_data_to_packet(buffer, password, strlen(password)+1);
}

short build_rs_update_ranking_packet(
	char *buffer, 
	long user_id, 
	struct player_stats *stats_delta)
{
	build_empty_header(buffer, _rs_update_ranking_packet);
	append_data_to_packet(buffer, &user_id, sizeof(long));
	return append_data_to_packet(buffer, stats_delta, sizeof(struct player_stats));
}

short build_rs_update_room_data_packet(
	char *buffer, 
	short player_count,
	short game_count)
{	
	build_empty_header(buffer, _rs_update_room_data_packet);
	append_data_to_packet(buffer, &player_count, sizeof(short));
	return append_data_to_packet(buffer, &game_count, sizeof(short));
}

short build_rs_public_announcement_packet(
	char * buffer,
	char * psz_message)
{
	build_empty_header(buffer, _rs_public_announcement_packet);
	return append_data_to_packet(buffer, psz_message, strlen(psz_message) + 1);
}

short build_rs_player_enter_room_packet(
	char * buffer, 
	long player_id, 
	long room_id)
{
	build_empty_header(buffer, _rs_player_enter_room_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
}

short build_rs_player_info_request_packet(
	char * buffer,
	unsigned long player_id,
	unsigned long requested_player_id)
{
	build_empty_header(buffer, _rs_player_info_request_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)&requested_player_id, sizeof(requested_player_id));
}

short build_rs_ban_player_packet(
	char * buffer,
	unsigned long player_id,
	long ban_duration)
{
	build_empty_header(buffer, _rs_ban_player_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)&ban_duration, sizeof(ban_duration));
}

short build_rs_score_game_packet(
	char * buffer,
	struct game_data * game)
{
	build_empty_header(buffer, _rs_score_game_packet);
	append_data_to_packet(buffer, (void *)&game->creator_player_id, sizeof(unsigned long));
	append_data_to_packet(buffer, (void *)&game->game_classification, sizeof(short));
	append_data_to_packet(buffer, (void *)&game->player_count, sizeof(short));
	return append_data_to_packet(buffer, (void *)game->players, 
		sizeof(struct player_game_data) * MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME);
}

short build_rs_player_leave_room_packet(
	char * buffer, 
	long player_id, 
	long room_id)
{
	build_empty_header(buffer, _rs_player_leave_room_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)&room_id, sizeof(room_id));
}

short build_rs_request_ranking_packet(
	char *buffer, 
	unsigned long player_id)
{
	build_empty_header(buffer, _rs_request_ranking_packet);
	return append_data_to_packet(buffer, &player_id, sizeof(unsigned long));
}

short start_building_rs_status_packet(
	char *buffer,
	short player_available_count,
	short players_in_game_count,
	short games_available_count,
	short games_in_progress_count,
	short guest_player_count) // need two byte pad
{
	short pad;

	build_empty_header(buffer, _rs_status_packet);
	append_data_to_packet(buffer, &player_available_count, sizeof(short));
	append_data_to_packet(buffer, &players_in_game_count, sizeof(short));
	append_data_to_packet(buffer, &games_available_count, sizeof(short));
	append_data_to_packet(buffer, &games_in_progress_count, sizeof(short));
	append_data_to_packet(buffer, &guest_player_count, sizeof(short));
	
	return append_data_to_packet(buffer, &pad, sizeof(short));
}

short add_player_to_rs_status_packet(
	char *buffer,
	unsigned long player_id)
{
	struct room_packet_header *header= (struct room_packet_header *) buffer;
	assert(header->type==_rs_status_packet);

	return append_data_to_packet(buffer, &player_id, sizeof(unsigned long));
}

static short build_empty_player_query_response_header(
	char * buffer,
	unsigned long player_id)
{
	struct room_packet_header * header = (struct room_packet_header *)buffer;
	int n = 0;

	header->type = _rs_player_query_response_packet;
	header->length = sizeof(struct room_packet_header);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	return append_data_to_packet(buffer, (void *)&n, sizeof(n));
}

short build_rs_player_query_packet(
	char * buffer, 
	long player_id, 
	unsigned long * buddy_list, 
	short order, 
	char * search_string)
{
	short unused;
	assert(search_string);

	build_empty_header(buffer, _rs_player_query_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	append_data_to_packet(buffer, (void *)buddy_list, sizeof(unsigned long) * MAXIMUM_BUDDIES);
	append_data_to_packet(buffer, (void *)&order, sizeof(order));
	append_data_to_packet(buffer, (void *)&unused, sizeof(unused));
	return append_data_to_packet(buffer, (void *)search_string, strlen(search_string) + 1);
}


short build_rs_player_query_response_packet(
	char * buffer,
	short type,
	unsigned long user_id,
	struct user_query_response * qr)
{
	struct player_query_response_segment * qs;
	struct player_query_response_header * qh;
	struct room_packet_header * header;
	int num_responses = 0;

	// ALAN Begin: unused variables 
	char /* * p_src, * p_dest, */ * p_segment;
	// ALAN End

	build_empty_player_query_response_header(buffer, user_id);
	header = (struct room_packet_header *)buffer;

	p_segment = (char *)buffer;
	p_segment += sizeof(struct room_packet_header);
	qh = (struct player_query_response_header *)p_segment;
	qh->type = type;

	p_segment += sizeof(struct player_query_response_header);
	if (qr)
	{
		while (qr->match_score && num_responses < MAXIMUM_PLAYER_SEARCH_RESPONSES)
		{
			if (qr->aux_data.player_id == user_id)
			{
				qr++;
				continue;
			}

			qs = (struct player_query_response_segment *)p_segment;
			qs->room_id = qr->aux_data.room_id;
			qs->aux_data = qr->aux_data;
			memcpy(qs->player_data, qr->player_data, MAXIMUM_PACKED_PLAYER_DATA_LENGTH);
			header->length += sizeof(struct player_query_response_segment);
			p_segment += sizeof(struct player_query_response_segment);
			qr++;
			num_responses++;
		}
	}

	qh->number_of_responses = num_responses;
	return ((struct room_packet_header *)(buffer))->length;
}

short build_rs_update_player_information_packet(
	char * buffer,
	unsigned long player_id,
	char * player_information)
{
	char * p;
	short s;

	build_empty_header(buffer, _rs_update_player_information_packet);
	append_data_to_packet(buffer, (void *)&player_id, sizeof(player_id));
	p = player_information;
	s = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, s);
	p += s;
	s = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, s);
	p += s;
	s = strlen(p) + 1;
	append_data_to_packet(buffer, (void *)p, s);
	p += s;
	s = strlen(p) + 1;
	return append_data_to_packet(buffer, (void *)p, s);
}

/* ------------- byteswapping */
#ifdef little_endian

static byte_swap_code _bs_rs_public_announcement_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte, // type, length
	_end_bs_array
};

static byte_swap_code _bs_rs_login_successful_packet[]= { 
	_begin_bs_array, 1,	
		_2byte,	_2byte, // type, length
		_2byte, // identifier
		_2byte, // game_type
		_2byte, // player_data_size
		_2byte, // game_data_size
		_2byte, // ranked_room
		_2byte, // tournament_room
		_4byte, _4byte, _4byte, _4byte,		// normal caste breakpoints
		_4byte, _4byte, _4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte, _4byte,
		_4byte, _4byte,
		_4byte,
		_4byte,
		_4byte,
	_end_bs_array 
};

static byte_swap_code _bs_game_rank_data[] = {
	_begin_bs_array, 1,
		MAXIMUM_PLAYER_NAME_LENGTH + 1,
		_4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_rank_update_packet_prefix[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte, _4byte, _4byte,		// normal caste breakpoints
		_4byte, _4byte, _4byte, _4byte,
		_4byte, _4byte,
		_4byte, _4byte, _4byte,
		_4byte, _4byte,
		_4byte,
		_4byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_login_failure_packet[]= {	
	_begin_bs_array, 1,	
		_2byte,	_2byte, // type, length
		_2byte, // short (reason)
	_end_bs_array 
};

static byte_swap_code _bs_long[] = {
	_begin_bs_array, 1,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_client_ranking_packet[]= { 
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
		_4byte, // player_id
		_2byte, // flags
		_2byte, // padding
		_4byte, // score
		_4byte, // points killed
		_4byte, // points lost
		_4byte, // units killed
		_4byte, // units lost
		_4byte, // first_place wins
		_4byte, // games played
		_2byte, // caste
		_2byte, // padding
	_end_bs_array 
};

static byte_swap_code _bs_rs_motd_changed_packet[]= { 
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
	_end_bs_array 
};

static byte_swap_code _bs_rs_send_status_packet[]= { 
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
	_end_bs_array 
};

static byte_swap_code _bs_rs_login_packet[]= {	
	_begin_bs_array, 1,	
		_2byte,	_2byte, // type, length
		_2byte, // short (port)
		_2byte, // short identifier
	_end_bs_array 
};

static byte_swap_code _bs_rs_update_ranking_packet[]= {	
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
		_4byte, // user_id
			_4byte, // games_played
			_4byte, // points killed
			_4byte, // points lost
			_4byte, // units killed
			_4byte, // units lost
			_2byte, // first_place wins
			_2byte, // last_place wins
			_4byte, // time at first login
			_2byte, // caste
			_2byte, // default room
	_end_bs_array 
};

static byte_swap_code _bs_rs_update_room_data_packet[]= {	
	_begin_bs_array, 1,	
		_2byte,	_2byte, // type, length
		_2byte, // short (player_count)
		_2byte, // short (game_count)
	_end_bs_array 
};

static byte_swap_code _bs_rs_request_ranking_packet[]= { 
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
		_4byte, // player_id
	_end_bs_array 
};

static byte_swap_code _bs_rs_status_packet[]= {
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
		_2byte, _2byte, _2byte, _2byte, _2byte, _2byte, // player counts, pads, etc.
	_end_bs_array 
};

static byte_swap_code _bs_rs_room_list_packet_header[]= { 
	_begin_bs_array, 1,	
		_2byte, _2byte,	// type, length (header)
	_end_bs_array 
};

static byte_swap_code _bs_rs_room_info[]= {
	_begin_bs_array, 1,	
		// info...
		_2byte, _2byte,	// room_id, player_count
		_4byte, // host
		_2byte, _2byte, // port, game_count
		_2byte, _2byte,	// room_type, unused
		_2byte, _2byte,	// unused, unused
		_2byte, _2byte,	// unused, unused
	_end_bs_array
};

static byte_swap_code _bs_rs_player_room_list_data[]= {
	_begin_bs_array, 1,	
		_2byte, _2byte, // country code, padding
		_2byte, _2byte, // min, max caste
		_2byte, _2byte, // ranked_room, tournament_room
	_end_bs_array 
};

static byte_swap_code _bs_rs_ping_packet[]={
	_begin_bs_array, 1,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_player_information_query_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,			// header
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_player_information_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,						// player_id
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,		// buddies
		_2byte, _2byte,				// order / country_code
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_update_buddy_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte,	_4byte,		// buddy_id, player_id, add
	_end_bs_array
};

static byte_swap_code _bs_rs_update_buddy_response_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
		_4byte, 4, _4byte, 4,
	_end_bs_array
};

static byte_swap_code _bs_rs_player_enter_room_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_player_leave_room_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_player_query_response_segment[] = {
	_begin_bs_array, 1,
		_4byte,
		_2byte, _2byte,
		_4byte, _4byte,
		_4byte, _2byte,
		_2byte, MAXIMUM_PACKED_PLAYER_DATA_LENGTH,
	_end_bs_array
};

static byte_swap_code _bs_header[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_player_query_response_header[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_player_query_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
		_4byte, _4byte, _4byte, _4byte,
		_4byte, _4byte, _4byte, _4byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_update_order_status_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_order_member[] = {
	_begin_bs_array, 1,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_update_player_information_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_player_info_request_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_bungie_net_player_score_datum[] = {
	_begin_bs_array, 1,
		_2byte, _2byte, _2byte, _2byte,			// games played, wins, losses, ties
		_4byte, _4byte,							// damage inflicted / damage received
		_2byte, _2byte, _2byte, _2byte,			// disconnects, pad, points, rank
		_2byte, _2byte,							// highest points / highest rank
		_4byte, 16,
	_end_bs_array
};


static byte_swap_code _bs_rs_player_info_reply_packet_prefix[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte, _4byte, _4byte,					// player id, admin, bungie emp
		_2byte, _2byte,							// order index, icon index
		_2byte, _2byte, _2byte, _2byte,			// primary color
		_2byte, _2byte, _2byte, _2byte,			// secondary color
	_end_bs_array
};

static byte_swap_code _bs_rs_ban_player_packet[] = {
	_begin_bs_array, 1, 
		_2byte, _2byte,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_rs_global_message_packet[] = {
	_begin_bs_array, 1,
		_2byte, _2byte,
		_4byte,
	_end_bs_array
};

static byte_swap_code _bs_score_game_prefix[] = {
	_begin_bs_array, 1,
		_4byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_player_game_data_prefix[] = {
	_begin_bs_array, 1,
		_4byte, _4byte,
	_end_bs_array
};

static byte_swap_code _bs_bungie_net_game_standings_prefix[] = {
	_begin_bs_array, 1,
		_2byte, _2byte, _4byte, 
		_4byte, _4byte, 
		_4byte,
		_2byte, _2byte,
	_end_bs_array
};

static byte_swap_code _bs_bungie_net_game_standings_team[] = {
	_begin_bs_array, 1,
		_2byte, _2byte, _2byte, _2byte,
		_4byte,
	_end_bs_array 
};

static byte_swap_code _bs_bungie_net_game_standings_player[] = {
	_begin_bs_array, 1, 
		_2byte, _2byte, _4byte,
		_4byte, _4byte, 
		_4byte, _4byte, 
		_4byte,
	_end_bs_array 
};

struct {
	byte_swap_code *code;
	short length;
} codes[]= {
	// from server
	{ _bs_rs_login_successful_packet, sizeof(struct room_packet_header)+sizeof(struct rs_login_successful_packet) }, // _rs_login_successful_packet
	{ _bs_rs_login_failure_packet, sizeof(struct room_packet_header)+sizeof(short) }, // _rs_login_failure_packet
	{ _bs_rs_client_ranking_packet, sizeof(struct room_packet_header)+sizeof(struct rs_client_ranking_packet) },
	{ _bs_rs_motd_changed_packet, sizeof(struct room_packet_header) },
	{ _bs_rs_send_status_packet, sizeof(struct room_packet_header) },
	{ _bs_rs_room_list_packet_header, sizeof(struct room_packet_header) },
	{ _bs_rs_player_information_packet, sizeof(struct room_packet_header) + sizeof(struct rs_player_information_packet) },
	{ _bs_rs_update_buddy_response_packet, sizeof(struct room_packet_header) + sizeof(struct rs_update_buddy_response_packet) },
	{ _bs_rs_global_message_packet, sizeof(struct room_packet_header) + sizeof(struct rs_global_message_packet) },
	{ _bs_rs_ping_packet, sizeof(struct room_packet_header) },

	// from client
	{ _bs_rs_login_packet, sizeof(struct room_packet_header)+2*sizeof(short) }, // rs= room_server
	{ _bs_rs_update_ranking_packet, sizeof(struct room_packet_header)+sizeof(struct rs_update_ranking_packet) }, // _rs_update_ranking_packet
	{ _bs_rs_update_room_data_packet, sizeof(struct room_packet_header)+sizeof(struct rs_update_room_data_packet) }, // _rs_update_room_data_packet
	{ _bs_rs_request_ranking_packet, sizeof(struct room_packet_header)+sizeof(struct rs_request_ranking_packet) },
	{ _bs_rs_status_packet, sizeof(struct room_packet_header)+sizeof(struct rs_status_packet) },
	{ _bs_rs_public_announcement_packet, sizeof(struct room_packet_header) },
	{ _bs_rs_player_information_query_packet, sizeof(struct room_packet_header) + sizeof(struct rs_player_information_query_packet) },
	{ _bs_rs_update_buddy_packet, sizeof(struct room_packet_header) + sizeof(struct rs_update_buddy_packet) },
	{ _bs_rs_player_enter_room_packet, sizeof(struct room_packet_header) + sizeof(struct rs_player_enter_room_packet) },
	{ _bs_rs_player_leave_room_packet, sizeof(struct room_packet_header) + sizeof(struct rs_player_leave_room_packet) },
	{ _bs_rs_player_query_packet, sizeof(struct room_packet_header) + sizeof(struct rs_player_query_packet) },
	{ _bs_rs_update_player_information_packet, sizeof(struct room_packet_header) + sizeof(struct rs_update_player_information_packet) },	// update_player_information_packet
	{ _bs_rs_player_info_request_packet, sizeof(struct room_packet_header) + sizeof(struct rs_player_info_request_packet) },
	{ _bs_rs_ban_player_packet, sizeof(struct room_packet_header) + sizeof(struct rs_ban_player_packet) }
};
#define NUMBER_OF_BYTESWAPPING_CODES (sizeof(codes)/sizeof(codes[0]))

void byte_swap_rs_rank_update_packet(
	char * buffer)
{
	char * p;
	short index;

	p = buffer;
	byte_swap_data("bungie net rank update prefix", p, sizeof(struct rs_rank_update_packet) - 
		(sizeof(struct game_rank_data) * NUMBER_OF_GAME_RANK_DATAS_IN_OVERALL_RANKING_DATA), 1,
		_bs_rs_rank_update_packet_prefix);

	p += sizeof(struct rs_rank_update_packet) - 
		(sizeof(struct game_rank_data) * NUMBER_OF_GAME_RANK_DATAS_IN_OVERALL_RANKING_DATA);

	byte_swap_data("bungie net game rank data", p, sizeof(long), 1, _bs_long);

	for (index = 0; index < NUMBER_OF_GAME_RANK_DATAS_IN_OVERALL_RANKING_DATA; ++index)
	{
		byte_swap_data("bungie net game rank data", p, sizeof(struct game_rank_data), 1, 
			_bs_game_rank_data);
		p += sizeof(struct game_rank_data);
	}
}

void byte_swap_bungie_net_game_standings(
	char * buffer)
{
	char * p;
	short index;

	p = buffer;
	byte_swap_data("bungie net game standings prefix", p, sizeof(struct bungie_net_game_standings) - 
		(sizeof(struct bungie_net_game_standings_team) * MAXIMUM_TEAMS_PER_MAP) - 
		(sizeof(struct bungie_net_game_standings_player) * MAXIMUM_PLAYERS_PER_MAP), 1, 
		_bs_bungie_net_game_standings_prefix);

	p += sizeof(struct bungie_net_game_standings) - 
		(sizeof(struct bungie_net_game_standings_team) * MAXIMUM_TEAMS_PER_MAP) - 
		(sizeof(struct bungie_net_game_standings_player) * MAXIMUM_PLAYERS_PER_MAP);
	for (index = 0; index < MAXIMUM_TEAMS_PER_MAP; ++index)
	{
		byte_swap_data("bungie net game standings team", p, 
			sizeof(struct bungie_net_game_standings_team), 1, 
			_bs_bungie_net_game_standings_team);
		p += sizeof(struct bungie_net_game_standings_team);
	}

	for (index = 0; index < MAXIMUM_PLAYERS_PER_MAP; ++index)
	{
		byte_swap_data("bungie net game standings player", p,
			sizeof(struct bungie_net_game_standings_player), 1,
			_bs_bungie_net_game_standings_player);
		p += sizeof(struct bungie_net_game_standings_player);
	}
}

void byte_swap_rs_score_game_packet(
	char * buffer,
	boolean outgoing)
{
	char * p;
	short index;

	p = buffer;
	byte_swap_data("score game packet header", p, sizeof(struct room_packet_header), 
		1, _bs_header);

	p += sizeof(struct room_packet_header);
	byte_swap_data("score game packet prefix", p, sizeof(struct rs_score_game_packet) - 
		(sizeof(struct player_game_data) * MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME),
		1, _bs_score_game_prefix);

	p += sizeof(struct rs_score_game_packet) - 
		(sizeof(struct player_game_data) * MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME);
	for (index = 0; index < MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME; ++index)
	{
		byte_swap_data("player game data prefix", p, sizeof(struct player_game_data) -
			sizeof(struct bungie_net_game_standings), 1, _bs_player_game_data_prefix);

		p += sizeof(struct player_game_data) - sizeof(struct bungie_net_game_standings);
		byte_swap_bungie_net_game_standings(p);

		p += sizeof(struct bungie_net_game_standings);
	}
}

void byte_swap_rs_update_order_status_packet(
	char * buffer,
	boolean outgoing)
{
	char * p;
	short index;
	short member_count= 0;

	p = buffer;
	byte_swap_data("update order status packet", p, sizeof(struct room_packet_header), 
		1, _bs_header);
	p += sizeof(struct room_packet_header);

	if (outgoing)
	{
		member_count = ((struct rs_update_order_status_packet *)(p))->member_count;
	}

	byte_swap_data("update order status packet", p, sizeof(struct rs_update_order_status_packet),
		1, _bs_rs_update_order_status_packet);

	if (!outgoing)
	{
		member_count = ((struct rs_update_order_status_packet *)(p))->member_count;
	}

	p += sizeof(struct rs_update_order_status_packet);
	for (index = 0; index < member_count; index++)
	{
		byte_swap_data("update order status packet", p, sizeof(struct order_member),
			1, _bs_order_member);
		p += sizeof(struct order_member);
	}
}

void byte_swap_rs_player_query_response_packet(
	char * buffer, 
	boolean outgoing)
{
	char * byte_swap_me;
	int segment_index, number_of_responses= 0;
	struct player_query_response_header * qh;

	byte_swap_me = buffer;
	byte_swap_data("query response packet", byte_swap_me, sizeof(struct room_packet_header),
		1, _bs_header);
	byte_swap_me += sizeof(struct room_packet_header);

	qh = (struct player_query_response_header *)byte_swap_me;

	if (outgoing)
	{
		number_of_responses = qh->number_of_responses;
	}

	byte_swap_data("query response packet", byte_swap_me, sizeof(struct player_query_response_header),
		1, _bs_player_query_response_header);

	if (!outgoing)
	{
		number_of_responses = qh->number_of_responses;
	}

	byte_swap_me += sizeof(struct player_query_response_header);

	for (segment_index = 0; segment_index < number_of_responses; segment_index++)
	{
		byte_swap_data("player search response segment", byte_swap_me, sizeof(struct player_query_response_segment),
			1, _bs_player_query_response_segment);

		byte_swap_me += sizeof(struct player_query_response_segment);
	}
}

void byte_swap_rs_player_info_reply_packet(
	char * buffer, 
	boolean outgoing)
{
	char * p;
	short n;
	
	p = buffer;
	byte_swap_data("player info reply", p, sizeof(struct room_packet_header) + SIZE_OF_PLAYER_INFO_REPLY_PREFIX,
		1, _bs_rs_player_info_reply_packet_prefix);
	p += sizeof(struct room_packet_header) + SIZE_OF_PLAYER_INFO_REPLY_PREFIX;

	for (n = 0; n < NUMBER_OF_SCORES_IN_PLAYER_INFO; ++n)
	{
		byte_swap_data("player info reply", p, sizeof(struct bungie_net_player_score_datum), 
			1, _bs_bungie_net_player_score_datum);
		p += sizeof(struct bungie_net_player_score_datum);
	}
}

boolean byteswap_room_packet(
	char *buffer, 
	boolean outgoing)
{
	short type, length;
	struct room_packet_header *header= (struct room_packet_header *) buffer;
	boolean ret= FALSE;

	type= header->type;
	length= header->length;
	if(!outgoing) 
	{
		type= SWAP2(type);
		length= SWAP2(length);
	}
	
	if (type >=0 && type < NUMBER_OF_PACKET_TYPES)
	{
		ret= TRUE;
		if (type == _rs_player_query_response_packet)
		{
			byte_swap_rs_player_query_response_packet(buffer, outgoing);
		}
		else if (type == _rs_update_order_status_packet)
		{
			byte_swap_rs_update_order_status_packet(buffer, outgoing);
		}
		else if (type == _rs_score_game_packet)
		{
			byte_swap_rs_score_game_packet(buffer, outgoing);
		}
		else if (type == _rs_player_info_reply_packet)
		{
			byte_swap_rs_player_info_reply_packet(buffer, outgoing);
		}
		else if (type == _rs_rank_update_packet)
		{
			byte_swap_rs_rank_update_packet(buffer);
		}
		else
		{
			byte_swap_data("room packet", buffer, codes[type].length, 1, codes[type].code);
		}

		// swap the player ids!
		switch(type)
		{
			case _rs_status_packet:
				{
					short player_count;
					short offset;
				
					offset= sizeof(struct room_packet_header)+sizeof(struct rs_status_packet);
					length-= offset;
					player_count= length/sizeof(unsigned long);
					assert(player_count*sizeof(unsigned long)==length);
					byte_swap_memory("player ids in room packet", (char *) buffer+offset, player_count, _4byte);
				}
				break;
			
			case _rs_room_list_packet:
				{
					short room_count,n;
					short offset;
					char * p;
					
					offset= sizeof(struct room_packet_header);
					length-= offset;
					p = buffer;
					p += offset;
					room_count= length/(sizeof(struct player_room_list_data)+sizeof(struct room_info));
	
					for (n= 0; n<room_count; ++n)
					{
						byte_swap_data("room packet data", 
							p, sizeof(struct room_info), 
							1, _bs_rs_room_info);
						p+= sizeof(struct room_info);
						byte_swap_data("room packet data",
							p, sizeof(struct player_room_list_data),
							1, _bs_rs_player_room_list_data);
						p+= sizeof(struct player_room_list_data)-sizeof(struct room_info);
					}
				}
				break;
		}
	}

	return ret;
}
#else
void byteswap_room_packet(
	char *buffer, 
	boolean outgoing)
{
	UNUSED_PARAMETER(buffer);
	UNUSED_PARAMETER(outgoing);
	
	return;
}
#endif

#include <sys/types.h>
#include <sys/socket.h>

boolean send_room_packet(
	int socket, 
	char *buffer, 
	short length)
{
	int last_sent;
	int left_to_send;
	// ALAN Begin: unused variable
//	boolean success= TRUE;
	// ALAN End
	char *p = buffer;
	
	// byteswap
	byteswap_room_packet(buffer, TRUE);
	left_to_send= length;
	while (left_to_send)
	{
		last_sent = send(socket, p, left_to_send, 0);
		if (last_sent==NONE)
			return FALSE;
		left_to_send-= last_sent;
		p+= last_sent;
	}
	byteswap_room_packet(buffer, FALSE);
	
	return TRUE;
}

/* -------------------- shouldn't really be in here, but we have no better place for it. */
static int compare_player_room_list(struct player_room_list_data *player1, struct player_room_list_data *player2);

boolean test_tournament_room(
	char * login_name,
	short tournament_room)
{
	FILE * pf;
	char sz[1024];

	if (!tournament_room)
		return TRUE;

	switch (tournament_room)
	{
		case 1:
			sprintf(sz, "%s/troom1", get_metaserver_root_dir());
			pf = fopen(sz, "r");
			break;
		case 2:
			sprintf(sz, "%s/troom2", get_metaserver_root_dir());
			pf = fopen(sz, "r");
			break;
		case 3:
			sprintf(sz, "%s/troom3", get_metaserver_root_dir());
			pf = fopen(sz, "r");
			break;
		case 4:
			sprintf(sz, "%s/troom4", get_metaserver_root_dir());
			pf = fopen(sz, "r");
			break;
		case 5:
			sprintf(sz, "%s/troom5", get_metaserver_root_dir());
			pf = fopen(sz, "r");
			break;
		case 6:
			sprintf(sz, "%s/troom6", get_metaserver_root_dir());
			pf = fopen(sz, "r");
			break;
		default:
			return FALSE;
	}
	
	if(pf == NULL)
		return TRUE;

	{
		char buf0[MAXIMUM_PLAYER_NAME_LENGTH+1];
		char buf1[MAXIMUM_PLAYER_NAME_LENGTH+1];

		strcpy(buf0, login_name);
		while (fgets(sz, sizeof(sz), pf) != NULL)
		{
			// strip out trailing EOL and CR
			char * p = sz;
			while ((*p != '\0') && (*p != '\n') && (*p != '\r'))
			{
				p++;
			}
			if (*p == '\n' || *p == '\r')
				*p = '\0';

			strcpy(buf1, sz);
			strlwr(buf0);
			strlwr(buf1);

			if (!strcmp(buf0, buf1))
			{
				fclose(pf);
				return TRUE;
			}
		}
	}

	fclose(pf);
	return FALSE;
}

//  list passed doesn't have to worry about versions...
short find_client_available_rooms(
	short player_caste, 
	short country_code,
	char * login_name,
	struct player_room_list_data *rooms,
	short count,
	boolean allow_all_rooms)
{
	struct player_room_list_data best_rooms[MAXIMUM_ROOMS];
	short actual_count= 0;
	short index;

	memset(best_rooms, 0, sizeof(struct player_room_list_data)*MAXIMUM_ROOMS);
	allow_all_rooms = FALSE;
	
	// only copy in the available rooms (by caste)
	for(index= 0; index<count; ++index)
	{
		if (allow_all_rooms || (rooms[index].minimum_caste<=player_caste && player_caste<=rooms[index].maximum_caste))
		{
			// if this is an american room, or your country code matches the rooms country code...
			if(rooms[index].country_code==0 || rooms[index].country_code==country_code)
			{
				if (test_tournament_room(login_name, rooms[index].tournament_room))
					// add room to the list
					memcpy(&best_rooms[actual_count++], &rooms[index], sizeof(struct player_room_list_data));
			}
		}	
	}

	// now sort the list...
	qsort(best_rooms, actual_count, sizeof(struct player_room_list_data),
		(int (*)(const void *, const void *)) compare_player_room_list);

	count= MIN(actual_count, MAXIMUM_ROOMS_PER_PLAYER);
	//memcpy(rooms, best_rooms, actual_count*sizeof(struct player_room_list_data));
	memcpy(rooms, best_rooms, count*sizeof(struct player_room_list_data));

	return count;
}

static int compare_player_room_list(
	struct player_room_list_data *room1,
	struct player_room_list_data *room2)
{
	int delta;

	// sort by 1) country code (yours first), 2) maximum caste
	delta= room2->country_code - room1->country_code;
	if(!delta)
	{
		if (room2->tournament_room && !room1->tournament_room)
			delta++;
		else if (!room2->tournament_room && room1->tournament_room)
			delta--;

		if (!delta)
		{
			delta= room2->minimum_caste - room1->minimum_caste;
			if(!delta)
			{
				// finally, guarantee constant
//				delta= room1->info.room_id - room2->info.room_id;
				delta= room2->info.room_id - room1->info.room_id;
			}
		}
	}
	
	return delta;
}

/* ------------- local code */
static short build_empty_header(
	char *buffer,
	short type)
{
	struct room_packet_header *header= (struct room_packet_header *) buffer;

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
	struct room_packet_header *header= (struct room_packet_header *) buffer;
	char *dest= (buffer+header->length);

	/* Fill in the header */	
	memcpy(dest, data, data_length);
	header->length+= data_length;

	return header->length;
}
