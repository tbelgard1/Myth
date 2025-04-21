/*
	metaserver_packets.h
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

enum {
	MAXIMUM_METASERVER_APPLICATION_NAME= 31,
	MAXIMUM_METASERVER_BUILD_DATE= 31,
	MAXIMUM_METASERVER_BUILD_TIME= 31,
	NUMBER_OF_SCORING_DATUMS_IN_PLAYER_INFO_PACKET= 36
};

/* Circular queues */
struct circular_buffer {
	char *buffer;
	short size;
	short read_index;
	short write_index;
};

/* Macros for circular queues */
#define CIRCULAR_BUFFER_WRITTEN_SIZE(b) ((b)->read_index<=(b)->write_index) ? ((b)->write_index-(b)->read_index) : ((b)->size-(b)->read_index+(b)->write_index)
#define INCREMENT_CIRCULAR_BUFFER_READ_INDEX(b) if(++(b)->read_index>=(b)->size) (b)->read_index= 0

/* ---------- packet stuff */
enum {
	PACKET_IDENTIFIER = 0xDEAD,
	METASERVER_PACKET_VERSION = 1,
	FIRST_CLIENT_PACKET_ID = 100,
	FIRST_BOTH_PACKET_ID = 200,

	// These are sent from the server
	_room_list_packet = 0,
	_player_list_packet,
	_game_list_packet,
	_server_message_packet,
	_url_packet,
	_data_chunk_packet,
	_password_challenge_packet,
	_user_successful_login_packet,
	_set_player_data_from_metaserver_packet,
	_room_login_successful_packet,
	_message_of_the_day_packet,
	_patch_packet,
	_send_versions_packet,
	_game_list_pref_packet,
	_player_search_list_packet,
	_buddy_list_packet,
	_order_list_packet,
	_player_info_packet,
	_update_info_packet,
	_update_player_buddy_list_packet,
	_update_order_member_list_packet,
        _you_just_got_blammed_sucka_packet,
	
	// These are sent from the client
	_login_packet = FIRST_CLIENT_PACKET_ID,
	_room_login_packet,
	_logout_packet,
	_set_player_data_packet,
	_create_game_packet,
	_remove_game_packet,
	_change_room_packet,
	_set_player_mode_packet,
	_data_chunk_reply_packet,
	_password_response_packet,
	_request_full_update_packet,
	_game_player_list_packet,
	_game_score_packet,
	_reset_game_packet,
	_start_game_packet,
	_version_control_packet,
	_game_search_query_packet,
	_player_search_query_packet,
	_buddy_query_packet,
	_order_query_packet,
	_update_buddy_packet,
	_player_info_query_packet,
	_update_player_information_packet,
	
	// Sent by both
	_room_broadcast_packet = FIRST_BOTH_PACKET_ID, // like a chat packet. (We don't touch the data.)
	_directed_data_packet, // like a chat packet to one person
	_keepalive_packet, // ping type packets. (server sends, client echos)

	// _MYTHDEV Begin
    _session_key_packet,
	// _MYTHDEV End

	NUMBER_OF_METASERVER_PACKETS
};

/* structures for the new binary system */
struct packet_header {
	word packet_identifier;
	short type;
	long length;
};

struct update_buddy_packet {
	long buddy_id;
	boolean add;
};

struct update_player_buddy_list_packet {
	struct buddy_entry buddies[MAXIMUM_BUDDIES];	
};

struct update_order_member_list_packet {
	long member_count;
};

struct version_control_packet {
	long platform_type;					// the users platform
	long application_version;			// the users application version
	long patch_number;					// the users latest patch number
	long bnupdate_version;
};

// only a header, information is generated
// by server when one is received
// struct buddy_query_packet {
// };

struct player_info_query_packet {
	long player_id;
};

struct player_info_packet {
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
	struct overall_ranking_data overall_rank_data;
	// char * login
	// char * name
	// char * order_name
	// char * description
};	

struct game_score_packet {
	unsigned long game_id;
	struct bungie_net_game_standings standings;
};

struct update_info_packet {
	long size;				// length in bytes of update
	long host;
	short port;
	short unused_short;
};

struct game_search_query_packet {
	short s_game_type;
	short s_game_scoring;
	short veterans;
	short unit_trading;
	short teams;
	short alliances;
	short enemy_visibility;
	short unused;
	// char * sz_game_name
	// char * sz_map_name
};

enum { // login flags
	_reset_player_data_bit,
	NUMBER_OF_LOGIN_FLAGS,
	
	_reset_player_data_flag= FLAG(_reset_player_data_bit)
};

// if packet->type==_login_packet
struct login_data {
	short metaserver_version;
	short platform_type;
	long flags;
	long user_id;
	short max_authentication;
	short player_data_size;
	char application[MAXIMUM_METASERVER_APPLICATION_NAME+1];
	char build_date[MAXIMUM_METASERVER_BUILD_DATE+1];
	char build_time[MAXIMUM_METASERVER_BUILD_TIME+1];
	char username[MAXIMUM_METASERVER_USERNAME+1];
	char player_data[MAXIMUM_METASERVER_PLAYER_DATA_SIZE];
	short country_code;
	short padding;
};

// if packet->type==_room_login_packet
struct room_login_data { 
	authentication_token token;
	char name[MAXIMUM_METASERVER_USERNAME+1];
};

// if packet->type==_room_list_packet

//struct url_packet {
	// long code;
	// message
	// url
//};

enum {
	_room_list= 0,
	_game_list,
	_player_list,
	NUMBER_OF_LIST_TYPES
};

enum {
	_requires_url,
	_requests_url,
	_request_url_at_exit,
	NUMBER_OF_URL_TYPES
};

struct request_list_packet {
	short list_desired;
};

struct server_message_packet {
	long error_code;
	char message[128];
};

struct password_challenge_packet {
	short authentication_type;
	unsigned char salt[MAXIMUM_SALT_SIZE];
};

struct user_successful_login_packet {
	long user_id;
	short order;
	short unused_short;
	authentication_token token;
};

struct create_game_packet {
	// header
	short port;
	short order;
	// game data
};

enum {
	_player_mode_listening,
	_player_mode_deaf,
	NUMBER_OF_PLAYER_MODES
};

struct set_player_mode_packet {
	short mode;
};

struct password_response_packet {
	char encrypted_password[MAXIMUM_METASERVER_PASSWORD+1];
};

struct start_game_packet {
	long game_time_in_seconds;
	long unused[2];
};

/* Prototypes */
/* -------- server packets */
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
	char * strings);

short start_building_room_packet(char *buffer);
short add_room_data_to_packet(char *buffer, struct room_info *room);

short start_building_player_list_packet(char *buffer);
short add_player_data_to_packet(char *buffer, 
	struct metaserver_player_aux_data *player_aux_data,
	void *player_data, short length);

short start_building_player_search_list_packet(char *buffer);
short add_player_data_to_search_packet(char *buffer, long room_id, struct metaserver_player_aux_data 
	*player_aux_data, void *player_data);

short start_building_game_list_packet(char *buffer);
short add_game_data_to_packet(char *buffer, struct metaserver_game_aux_data *aux_data,
	void *game_data, short game_data_length);

short build_server_message_packet(char *buffer, short error_code);

short build_url_packet(char *buffer, short error_code, char *message, char *url);
short build_room_login_successful_packet(char *buffer, unsigned long user_id, 
	short maximum_player_count);

short build_data_chunk_packet(char *buffer, struct data_chunk_identifier_data *id, void *data);

short build_password_challenge_packet(char *buffer, short authentication_type, unsigned char *salt);
short build_user_successful_login_packet(char *buffer, long user_id, short order, authentication_token *token);

short build_set_player_data_from_metaserver_packet(char *buffer, 
	struct metaserver_player_aux_data *player, char *data, short data_length);
short build_message_of_the_data_packet(char *buffer, char *message);

short build_patch_packet(char *buffer, word flags,short port,char *host,char *login,
	char *password,char *source_file,char *pre_download_prompt,char *download_prompt,
	char *post_download_prompt,char *failure_url);
short build_send_versions_packet(char *buffer);
short build_update_info_packet(char * buffer, long size, long host, short port);
short build_update_player_buddy_list_packet(char * buffer, struct buddy_entry * buddies);
short build_update_order_member_list_packet(char * buffer, long member_count, struct order_member * members);
short build_you_just_got_blammed_sucka_packet(char * buffer, char * message);

/* -------- client packets */
// _MYTHDEV Begin
short build_sessionkey_packet(char* buffer, char* my_public_key, int key_size);
// _MYTHDEV End
short build_player_info_query_packet(char * buffer, long player_id);
short build_login_packet(char *buffer, struct login_data *login);
short build_room_login_packet(char *buffer, authentication_token token, char version, char *name);
short build_logout_packet(char *buffer);
short build_set_player_data_packet(char *buffer, char *player_data, short player_data_size);
short build_game_packet(char *buffer, short game_port, short order, void *game_data, short game_data_length);
short build_reset_game_packet(char *buffer);
short build_remove_game_packet(char *buffer);
short build_change_room_packet(char *buffer, short new_room_id);
short build_set_player_mode_packet(char *buffer, short new_mode);
short build_data_chunk_reply_packet(char *buffer,  struct data_chunk_identifier_data *id);
short build_password_response_packet(char *buffer, char *encrypted_password);
short build_request_full_update_packet(char *buffer);
short build_request_full_update_packet(char *buffer);
short build_start_game_packet(char *buffer, long game_time_in_seconds);
short build_update_buddy_packet(char * buffer, long buddy_id, boolean add);
short build_order_query_packet(char * buffer, short order);
short build_buddy_query_packet(char * buffer);

short start_building_game_player_list_packet(char *buffer, long game_id);
short add_game_player_data_to_packet(char *buffer, struct player_list_packet_entry *entry);

short start_building_order_list_packet(char * buffer);
short add_player_data_to_order_list_packet(char *buffer, long room_id, 
	struct metaserver_player_aux_data *player_aux_data, void *player_data);

short start_building_buddy_list_packet(char * buffer);
short add_player_data_to_buddy_list_packet(char *buffer, long room_id,	
	struct metaserver_player_aux_data *player_aux_data,	void *player_data);

short build_game_score_packet(
	char * buffer, 
	unsigned long game_id, 
	struct bungie_net_game_standings * standings);

short build_version_control_packet(char *buffer, long platform_type, long application_version, 
	long patch_number, long bnupdate_version);
short build_game_search_query_packet(char *buffer, struct game_search_query *query);
short build_player_search_query_packet(char *buffer, char *string);

short build_update_player_information_packet(
	char * buffer, 
	char * city,
	char * state,
	char * country,
	char * personal);

/* ------- both */
short build_room_broadcast_packet(char *buffer, char *broadcast_data, short broadcast_data_length);
short build_directed_data_packet(char *buffer, unsigned long player_id, boolean echo, char *data, short data_length);
short build_keepalive_packet(char *buffer);

/* ------- aids to parsing */
boolean parse_network_stream(struct circular_buffer *buffer, struct packet_header *packet);

boolean byteswap_packet(char *buffer, boolean outgoing);



// ALAN Begin: added prototype to fix implicit declaration errors in "server_code.c"
short build_order_member_list_packet(
	char * buffer,
	long member_count,
	struct order_member * members);
// ALAN End

// ALAN Begin: added prototype to fix implicit declaration errors in "room_new.c"
short start_building_game_list_pref_packet(char * buffer);
// ALAN End
