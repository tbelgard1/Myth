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

#ifndef __USERS_H__
#define __USERS_H__

#include <sqlite3.h>

int open_users_db();
void close_users_db();
int add_user_sqlite(const char *username, const char *password);
int find_user_sqlite(const char *username);

// ALAN Begin: Added Headers
#include "stats.h"
// ALAN End

// THE FOLLOWING #define EFFECTIVELY LIMITS THE MAXIMUM MEMBERS OF AN ORDER 
// AS WELL AS THE MAXIMUM NUMBER OF BUDDIES (AND PLAYER SEARCH RESPONSES...)
#define	MAXIMUM_PLAYER_SEARCH_RESPONSES			 50

struct user_query
{
	char			string[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	unsigned long	buddy_ids[MAXIMUM_BUDDIES];
	short			order;
};

struct user_query_response
{
	long			match_score;
	struct metaserver_player_aux_data aux_data;
	char			player_data[MAXIMUM_PACKED_PLAYER_DATA_LENGTH];
};

boolean create_user_database(
	void);

boolean initialize_user_database(
	void);

void shutdown_user_database(
	void);

unsigned long get_user_count(
	void);

unsigned long get_first_player_in_order(
	short order_index,
	void ** key);

unsigned long get_next_player_in_order(
	void ** key);

boolean get_first_player_information(
	struct bungie_net_player_datum * player);

boolean get_next_player_information(
	struct bungie_net_player_datum * player);

boolean get_online_player_information(
	unsigned long player_id,
	struct bungie_net_online_player_data * player);

boolean get_player_information(
	char * login_name,
	unsigned long player_id,
	struct bungie_net_player_datum * player);

boolean update_player_information(
	char * login_name,
	unsigned long player_id,
	boolean logged_in_flag, 
	struct bungie_net_player_datum * player);

boolean new_user(
	struct bungie_net_player_datum * player);

void query_user_database(
	struct user_query * query,
	struct user_query_response ** query_response);

boolean is_player_online(
	unsigned long player_id);

short get_player_count_in_order(
	short order_index);

boolean get_user_game_data(
	unsigned long player_id,
	short game_index,
	struct player_stats * stats,
	word * flags);

boolean set_user_game_data(
	unsigned long user_index,
	short game_type,
	struct player_stats * stats);

boolean get_user_name(
	long user_index, 
	char * name);

boolean get_myth_user_data(
	unsigned long user_index,
	char * buffer,
	short * length);

void * build_rank_list(
	short game_type, 
	short maximum_users, 
	short * actual_users);

boolean set_user_as_bungie_admin(
	long user_index, 
	boolean set);

boolean set_user_password(
	long user_index, 
	char * password);


// ALAN Begin: added protypes to resolve implicit declaration errors in "main.c"
boolean set_myth_user_data(
	unsigned long user_index,
	char * buffer,
	short length);

void update_buddy_list(
	unsigned long player_id,
	unsigned long buddy_id,
	boolean add);
// ALAN End

#endif // __USERS_H__
