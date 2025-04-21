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

#ifndef __SERVER_CODE__
#define __SERVER_CODE__

#include "network_queues.h"

#include <sys/types.h>

struct client_data {
	int socket;
	unsigned long player_id;
	long host;
	short port;
	word flags;

	short order;

	struct player_stats stats;
	struct player_stats delta_stats;

	char version;
	short country_code;
	short caste;

	boolean player_is_guest;
	boolean player_data_set;
	boolean logged_in;
	boolean deaf;
	boolean sending_banner;
	boolean	in_ps_db;
	word metaserver_aux_data_flags;

	boolean player_is_admin;
	boolean player_is_bungie_employee;
	boolean account_is_kiosk;
	
	short login_attempts;
	char name[MAXIMUM_METASERVER_USERNAME+1];

	char player_data[MAXIMUM_METASERVER_PLAYER_DATA_SIZE];
	char personal_data[MAXIMUM_DESCRIPTION_LENGTH + 1];

	short player_data_size;
	short player_mode;

	struct buddy_entry buddies[MAXIMUM_BUDDIES];
	
	word game_flags;
	unsigned long game_id;
	short game_port;
	long game_start_ticks;
	long game_length_in_seconds;
	char game_data[MAXIMUM_METASERVER_GAME_DATA_SIZE];
	short game_data_size;

	long last_ping_ticks;
	short ping_count;

	long banner_offset;

	// _MYTHDEV Begin
    char    session_key[16];
    boolean valid_session_key;
	// _MYTHDEV End

	struct circular_queue outgoing_buffer;
	struct circular_queue incoming_buffer;
	
	struct client_data *next;
};

boolean add_client(int socket, long host, short port);
struct client_data *find_client_from_socket(int socket);
void delete_client(struct client_data *dead);
void closedown_client(struct client_data *client);

void build_client_writefds(fd_set *write_fds);

short current_client_count(void);

void refuse_client_room_full(int socket);

boolean ping_client(struct client_data *client);

struct client_data *find_first_client(void);
int get_client_socket(struct client_data *client);
struct client_data *find_next_client(struct client_data *client);

struct client_data * get_client_from_player_id(
	unsigned long player_id);

boolean set_client_ranking(unsigned long player_id, short flags, struct player_stats *stats);

boolean send_room_message_to_everyone(char *message);
boolean send_room_message_to_player(struct client_data *client, char *message);
void list_clients_logged_in(void);

boolean send_room_list_to_clients(struct player_room_list_data *available_rooms, short available_room_count);

unsigned long get_client_player_id(struct client_data *client);

void increment_player_transient_stats(unsigned long player_id, struct player_stats *s);

short build_room_status_packet(char *buffer);
short build_short_room_status_packet(char *buffer);

boolean copy_data_into_client_buffer(struct client_data *client, char *buffer, int length);
boolean handle_client_data(struct client_data *client, boolean *has_outgoing_data);

boolean send_message_to_player_in_room(unsigned long destination_player_id, char *message, short length);

unsigned long find_client_id_from_screen_name(char *name);
boolean get_client_screen_name(unsigned long player_id, char *name);

boolean client_is_admin(struct client_data *client);

boolean send_outgoing_data(struct client_data *client);

boolean get_logged_in_player_id_stats(unsigned long player_id, struct player_stats *stats);
boolean get_player_login_name(unsigned long player_id, char *login_name);

struct client_data *find_client_by_id(unsigned long player_id);
boolean boot_player(struct client_data *client, char* message);
boolean gag_player(struct client_data *client, boolean gag);

void update_userd_with_delta_stats(
	unsigned long player_id, 
	struct player_stats *a);

void set_client_player_information(
	long player_id, 
	struct buddy_entry buddies[MAXIMUM_BUDDIES], 
	short order,
	short country_code,
	boolean player_is_admin,
	boolean player_is_bungie_employee,
        boolean account_is_kiosk,
	char * login_name);

boolean update_client_buddy_list(
	long player_id, 
	struct buddy_entry * buddies);

boolean send_order_update_to_player(
	long player_id,
	short member_count,
	struct order_member * order_members);

boolean send_player_info_packet_to_player(
	struct rs_player_info_reply_packet * packet);

boolean is_player_admin(
	struct client_data * client);

boolean is_account_kiosk(
	struct client_data * client);

char *get_login_name_from_client(void *client);

// ALAN Begin: added prototype to resolve implicit declaration error in "games.c"
void report_game_to_userd(
	struct game_data * game);
// ALAN End

#endif // __SERVER_CODE__

