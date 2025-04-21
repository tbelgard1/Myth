/*
	room_packets.h
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

#ifndef __ROOM_PACKETS__
#define __ROOM_PACKETS__

#include "../users_new/bungie_net_player.h"
#include "../users_new/rank.h"
#include "games.h"

// ALAN Begin: Added Headers
#include "../users_new/users.h"
// ALAN End

//

#include "stats.h"

enum {
	ROOM_PASSWORD_SIZE= 16,
	ROOM_MAXIMUM_UPDATE_URL_SIZE= 256,
	ROOM_MAXIMUM_MOTD_SIZE= 256,
	MAXIMUM_ROOMS= 128, // 128*50==6400 users simultaneous
	MAXIMUM_ROOMS_PER_PLAYER= 24, // this is a client side limitation
};

enum {
	// from server
	_rs_login_successful_packet,
	_rs_login_failure_packet,
	_rs_client_ranking_packet,
	_rs_motd_changed_packet,
	_rs_send_status_packet,
	_rs_room_list_packet, // move up next time I recompile both.
	_rs_player_information_packet,
	_rs_update_buddy_response_packet,	
	_rs_global_message_packet,
	_rs_ping_packet,

	// from client
	_rs_login_packet,
	_rs_update_ranking_packet,
	_rs_update_room_data_packet,
	_rs_request_ranking_packet,
	_rs_status_packet,
	_rs_public_announcement_packet,
	_rs_player_information_query_packet,
	_rs_update_buddy_packet,
	_rs_player_enter_room_packet,
	_rs_player_leave_room_packet,
	_rs_player_query_packet,
	_rs_update_player_information_packet,
	_rs_player_info_request_packet,
	_rs_ban_player_packet,

	_rs_rank_update_packet,
	_rs_player_info_reply_packet,
	_rs_update_order_status_packet,
	_rs_player_query_response_packet,
	_rs_score_game_packet,
	NUMBER_OF_PACKET_TYPES
};

enum
{
	_player_search_query,
	_buddy_query,
	_order_query
};

struct room_packet_header {
	short type;
	short length;
};

struct rs_update_buddy_packet {
	long player_id;
	long buddy_id;
	boolean add;
};

struct rs_update_buddy_response_packet {
	long player_id;
	struct buddy_entry buddies[MAXIMUM_BUDDIES];
};

struct rs_global_message_packet {
	unsigned long player_id;
};

struct rs_player_information_query_packet {
	long player_id;
};

struct rs_player_information_packet {
	long player_id;
	struct buddy_entry buddies[MAXIMUM_BUDDIES];
	short order;
	short country_code;
	boolean player_is_admin;
	boolean player_is_bungie_employee;
        boolean account_is_kiosk;
	// char * login_name
};

struct rs_player_info_request_packet {
	unsigned long player_id;
	unsigned long requested_player_id;
};

#define	SIZE_OF_PLAYER_INFO_REPLY_PREFIX		32
#define	NUMBER_OF_SCORES_IN_PLAYER_INFO			36

struct rs_player_info_reply_packet {
	unsigned long player_id;
	boolean administrator_flag;
	boolean bungie_employee_flag;
	short order_index;
	short icon_index;
	struct rgb_color primary_color;
	struct rgb_color secondary_color;
	struct bungie_net_player_score_datum unranked_score_datum;
	struct bungie_net_player_score_datum ranked_score_datum;
	struct bungie_net_player_score_datum ranked_score_datum_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];
	struct bungie_net_player_score_datum order_unranked_score_datum;
	struct bungie_net_player_score_datum order_ranked_score_datum;
	struct bungie_net_player_score_datum order_ranked_score_datum_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];
	// char * login
	// char * name
	// char * order_name
	// char * description
};

struct rs_login_successful_packet {
	short identifier;
	short game_type;
	short player_data_size;
	short game_data_size;
	short ranked;
	short tournament_room;
	struct caste_breakpoint_data caste_breakpoints;
	// char *url_for_update
};

struct rs_rank_update_packet {
	struct caste_breakpoint_data caste_breakpoints;
	struct overall_ranking_data overall_rank_data;
};

struct rs_client_ranking_packet {
	unsigned long player_id;
	short flags;
	short padding;
	struct player_stats stats;
};

struct rs_login_packet {
	short port;
	short identifier; // NONE if we need the userd to tell us
	char password[32]; // variably lengthed...
};

struct rs_update_room_data_packet {
	short player_count;
	short game_count;
};

struct rs_update_ranking_packet {
	unsigned long user_id;
	struct player_stats stats;
};

struct rs_request_ranking_packet {
	unsigned long player_id;
};

struct rs_update_order_status_packet {
	unsigned long player_id;
	short member_count;
	short unused_short;
};

struct rs_status_packet { // room packet.
	short players_available_count;
	short players_in_game_count;
	short games_available_count;
	short games_in_progress_count;
	short guest_player_count;
	short pad;
	// long player_ids[players_available_count+players_in_game_count] array of player ids.
};

struct player_room_list_data {
	struct room_info info;
	short country_code;
	short padding;
	short minimum_caste;
	short maximum_caste;
	short ranked_room;
	short tournament_room;
};

struct rs_player_enter_room_packet
{
	unsigned long	player_id;
	long			room_id;
};

struct rs_player_leave_room_packet
{
	unsigned long	player_id;
	long			room_id;
};

struct rs_update_player_information_packet
{
	unsigned long	player_id;
};

struct rs_score_game_packet
{
	unsigned long creator_player_id;
	short game_classification;
	short player_count;
	struct player_game_data players[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
};

struct rs_ban_player_packet
{
	unsigned long player_id;
	long ban_duration;
};

struct rs_player_query_packet
{
	unsigned long player_id;
	unsigned long buddy_ids[MAXIMUM_BUDDIES];
	short order;
	short unused_short;
	// char string[] - player name ...
};

struct player_query_response_header
{
	unsigned long player_id;
	short type;
	short number_of_responses;
};

struct player_query_response_segment
{
	long room_id;
	struct metaserver_player_aux_data aux_data;
	char player_data[MAXIMUM_PACKED_PLAYER_DATA_LENGTH];
};

enum {
	_room_bad_password_error_code,
	_no_room_available_error_code
};

enum { // client_ranking_packet flags
	_client_is_bungie_admin_bit,
	NUMBER_OF_CLIENT_RANKING_PACKET_FLAGS,
	
	_client_is_bungie_admin_flag= FLAG(_client_is_bungie_admin_bit)
};

// from server
short build_rs_login_successful_packet(
	char * buffer, 
	short identifier, 
	short game_type, 
	short player_data_size, 
	short game_data_size, 
	short ranked_room, 
	short tournament_room, 
	struct caste_breakpoint_data * caste_breakpoints,
	char * url_for_version_update,
	char * motd);

short build_rs_login_failure_packet(
	char *buffer, 
	short code);

short build_rs_client_ranking_packet(
	char * buffer, 
	unsigned long player_id,
	short flags, 
	struct player_stats * stats);

short build_rs_motd_changed_packet(
	char * buffer, 
	char * motd);

short build_rs_send_status_packet(
	char * buffer);

short start_building_rs_room_packet(
	char * buffer);

short add_room_to_rs_room_packet(
	char * buffer, 
	struct player_room_list_data * room);

short build_rs_player_information_packet(
	char * buffer, 
	long player_id, 
	struct buddy_entry * buddies, 
	short order,
	boolean player_is_admin,
	boolean player_is_bungie_employee,
        boolean account_is_kiosk,
	short country_code,
	char * login_name);

short build_rs_update_buddy_response_packet(
	char * buffer, 
	long player_id, 
	struct buddy_entry * buddies);

short build_rs_update_order_status_packet(
	char * buffer, 
	unsigned long player_id,
	short member_count,
	struct order_member * order_members);

short build_rs_player_query_response_packet(
	char * buffer,
	short type,
	unsigned long user_id,
	struct user_query_response * qr);

short build_rs_player_info_reply_packet(
	char * buffer,
	unsigned long player_id,
	struct bungie_net_player_stats * stats);

short build_rs_global_message_packet(
	char * buffer, 
	unsigned long player_id,
	char * message);

short build_rs_caste_breakpoint_packet(
	char * buffer,
	struct caste_breakpoint_data * caste_breakpoints);

// accessors for server packets.
char * extract_url_for_update_from_rs_login_successful_packet(
	char *buffer);

char * extract_motd_from_rs_login_successful_packet(
	char *buffer);

char * extract_motd_from_rs_motd_changed_packet(
	char *buffer);
short build_rs_ping_packet(char *buffer);

// from client
short build_rs_login_packet(
	char * buffer, 
	short port, 
	short identifier, 
	char * password);

short build_rs_update_ranking_packet(
	char *buffer, 
	long user_id, 
	struct player_stats *stats_delta);

short build_rs_update_room_data_packet(
	char *buffer, 
	short player_count, 
	short game_count);

short build_rs_request_ranking_packet(
	char *buffer, 
	unsigned long player_id);

short start_building_rs_status_packet(
	char *buffer, 
	short players_available_count,
	short players_in_game_count, 
	short games_available_count, 
	short games_in_progress_count,
	short guest_player_count);

short add_player_to_rs_status_packet(
	char * buffer, 
	unsigned long player_id);

short build_rs_public_announcement_packet(
	char * buffer, 
	char * psz_message);

short build_rs_player_information_query_packet(
	char * buffer, 
	long player_id);

short build_rs_update_buddy_packet(
	char * buffer, 
	long player_id, 
	long buddy_id, 
	boolean add);

short build_rs_player_in_room_packet(
	char * buffer, 
	long player_id, 
	long room_id);

short build_rs_player_query_packet(
	char * buffer, 
	long player_id, 
	unsigned long * buddy_list, 
	short order, 
	char * search_string);

short build_rs_update_player_information_packet(
	char * buffer,
	unsigned long player_id,
	char * player_information);

short build_rs_player_info_request_packet(
	char * buffer,
	unsigned long player_id,
	unsigned long requested_player_id);

short build_rs_ban_player_packet(
	char * buffer,
	unsigned long player_id,
	long ban_duration);

short build_rs_score_game_packet(
	char * buffer, 
	struct game_data * game);

boolean byteswap_room_packet(
	char * buffer, 
	boolean outgoing);

boolean send_room_packet(
	int socket, 
	char * buffer, 
	short length);

short find_client_available_rooms(
	short player_caste, 
	short country_code, 
	char * login_name,
	struct player_room_list_data * rooms, 
	short count,
	boolean allow_all_rooms);


// ALAN Begin: added prototypes to fix implicit declarations errors in "server_code.c"
short build_rs_player_enter_room_packet(
	char * buffer, 
	long player_id, 
	long room_id);

short build_rs_player_leave_room_packet(
	char * buffer, 
	long player_id, 
	long room_id);
// ALAN End

// ALAN Begin: added prototypes to fix implicit declarations errors in "main.c"
short build_rs_rank_update_packet(
	char * buffer,
	struct caste_breakpoint_data * caste_breakpoints,
	struct overall_ranking_data * overall_rank_data);
// ALAN End

#endif // __ROOM_PACKETS__
