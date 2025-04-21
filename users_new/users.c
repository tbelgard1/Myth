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

#include "../utils/environment.h"

#ifdef __GNUC__
#warning "USING ENVIRONMENT.H — LINE SHOULD SHOW CORRECT CONST RETURN TYPE"
#endif
#include "cseries.h"
#include "metaserver_common_structs.h"
#include "rb_tree.h"
#include "sl_list.h"
#include "stats.h"
#include "byte_swapping.h"
#include "bungie_net_player.h"
#include "users.h"
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Added for SQLite3 support

// ALAN Begin: fixed multi-character character constant warnings
//#define	BUNGIE_NET_USER_DB_SIGNATURE		'PLAY'
#define	BUNGIE_NET_USER_DB_SIGNATURE		0x504c4159
// ALAN End

// SQLite3 user DB support
static sqlite3 *users_db = NULL;

int open_users_db(void) {
    printf("[DEBUG] open_users_db() - ENTRY\n");
    const char *db_path = NULL;
    printf("[DEBUG] Before get_users_db_file_name\n");
    db_path = get_users_db_file_name();
    printf("[DEBUG] After get_users_db_file_name, db_path: %s\n", db_path ? db_path : "(null)");

    printf("[DEBUG] Before sqlite3_open\n");
    int rc = sqlite3_open(db_path, &users_db);
    printf("[DEBUG] After sqlite3_open, rc=%d, users_db=%p\n", rc, (void*)users_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[ERROR] sqlite3_open failed: %s\n", users_db ? sqlite3_errmsg(users_db) : "users_db is NULL");
        printf("[DEBUG] Returning 0 from open_users_db (sqlite3_open failed)\n");
        return 0;
    }
    printf("[DEBUG] SQLite opened successfully\n");

    const char *sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password TEXT NOT NULL, created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
    char *errmsg = NULL;

    printf("[DEBUG] Before sqlite3_exec\n");
    rc = sqlite3_exec(users_db, sql, 0, 0, &errmsg);
    printf("[DEBUG] After sqlite3_exec, rc=%d\n", rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[ERROR] sqlite3_exec failed: %s\n", errmsg ? errmsg : "(no errmsg)");
        if (errmsg) sqlite3_free(errmsg);
        printf("[DEBUG] Returning 0 from open_users_db (sqlite3_exec failed)\n");
        return 0;
    }
    printf("[DEBUG] Table check complete\n");
    printf("[DEBUG] open_users_db() — returning 1\n");
    return 1;
}

    // Create table if not exists
    const char *sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password TEXT NOT NULL, created_at DATETIME DEFAULT CURRENT_TIMESTAMP);";
    char *errmsg = 0;


void close_users_db() {
    if (users_db) sqlite3_close(users_db);
}

// Stub: Add user
int add_user_sqlite(const char *username, const char *password) {
    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(users_db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// Stub: Find user
int find_user_sqlite(const char *username) {
    const char *sql = "SELECT id FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(users_db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_ROW;
}

struct order_member_list_data
{
	unsigned long player_id;
};

struct order_list_data
{
	short order_index;
	struct sl_list *member_list;
};

struct bungie_net_user_db_header
{
	unsigned long player_count;
	unsigned long unused[40];
};

struct bungie_net_user_db_entry
{
	unsigned long signature;
	struct bungie_net_player_datum player;
};

struct bungie_net_login_tree_data
{
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	unsigned long online_data_index;
	long fpos;
};

/* ALAN Begin: these functions defined but not used
static boolean lock_file_region(
	long offset, 
	long length,
	boolean write,
	boolean block);

static boolean unlock_file_region(
	long offset,
	long length);
// ALAN End  */

static boolean place_player_in_order(
	short order_index,
	unsigned long player_id);

static void remove_player_from_order(
	short order_index,
	unsigned long player_id);

static boolean order_list_new(
	void);

static struct sl_list_element *order_new(
	struct sl_list *order_list, 
	short order_index);

static void order_dispose(
	struct sl_list *order_list,
	struct sl_list_element *order_element);

static struct sl_list_element *order_member_new(
	struct sl_list *member_list, 
	unsigned long player_id);

static void order_member_dispose(
	struct sl_list *member_list,
	struct sl_list_element *member_element);

void order_member_list_dispose(
	struct sl_list *member_list);

static int order_list_comp_func(
	void *k0,
	void *k1);

static int order_member_list_comp_func(
	void *k0,
	void *k1);

static boolean add_entry_to_login_tree(
	struct bungie_net_player_datum * player,
	unsigned long fpos);

static int login_tree_comp_func(
	void * k0,
	void * k1);

static int compare_buddies(
	const void * p1, 
	const void * p2);

static void make_packed_player_data(
	char * player_data,
	short * length,
	struct bungie_net_player_datum * player);

static void refresh_response_list(
	void);

static void add_player_to_response_list(
	struct bungie_net_online_player_data * opd);

static char * ci_pn_match(
	char * s0,
	char * s1);

static struct rb_tree bungie_net_login_tree;
static struct sl_list * order_list;

static struct bungie_net_online_player_data * online_player_data;
static int fd_user_db = -1;
static unsigned long total_players;
static struct user_query_response response_list[MAXIMUM_PLAYER_SEARCH_RESPONSES];

// --- SQLite3-based user database logic only ---

// Open users.db and create tables if needed
int initialize_user_database(void) {
    return open_users_db();
}

void shutdown_user_database(void) {
    close_users_db();
}

// Register a new user (returns TRUE on success)
boolean new_user(struct bungie_net_player_datum *player) {
    // Check if user exists
    if (find_user_sqlite(player->login)) return FALSE;
    // Insert user
    if (!add_user_sqlite(player->login, player->password)) return FALSE;
    // TODO: Add additional fields (stats, etc.)
    return TRUE;
}

// Find user by login (returns TRUE if exists)
boolean get_player_information(char *login_name, unsigned long player_id, struct bungie_net_player_datum *player) {
    if (!login_name || !player) return FALSE;
    // Query user by username
    const char *sql = "SELECT username, password FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(users_db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return FALSE;
    sqlite3_bind_text(stmt, 1, login_name, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        strcpy(player->login, (const char *)sqlite3_column_text(stmt, 0));
        strcpy(player->password, (const char *)sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);
        return TRUE;
    }
    sqlite3_finalize(stmt);
    return FALSE;
}

// Set user password (returns TRUE on success)
boolean set_user_password(long user_index, char *password) {
    // user_index is not used in SQL version, use login instead
    // This is a stub; should be updated to use username
    return FALSE;
}

// Add more SQL-based CRUD as needed for stats, buddies, orders, etc.

// --- STUBS TO FIX LINKER ERRORS ---
unsigned long get_user_count(void) {
    // TODO: Implement actual logic
    return 0;
}

boolean update_player_information(char * login_name, unsigned long player_id, boolean logged_in_flag, struct bungie_net_player_datum * player) {
    // TODO: Implement actual logic
    return FALSE;
}

boolean is_player_online(unsigned long player_id) {
    // TODO: Implement actual logic
    return FALSE;
}

short get_player_count_in_order(short order_index) {
    // TODO: Implement actual logic
    return 0;
}

unsigned long get_first_player_in_order(short order_index, void **key) {
    // TODO: Implement actual logic
    return 0;
}

unsigned long get_next_player_in_order(void **key) {
    // TODO: Implement actual logic
    return 0;
}

boolean get_online_player_information(unsigned long player_id, struct bungie_net_online_player_data *player) {
    // TODO: Implement actual logic
    return FALSE;
}

void query_user_database(struct user_query *query, struct user_query_response **query_response) {
    // TODO: Implement actual logic
    *query_response = NULL;
}

void update_buddy_list(unsigned long player_id, unsigned long buddy_id, boolean add) {
    // TODO: Implement actual logic
}

boolean get_first_player_information(struct bungie_net_player_datum *player) {
    // TODO: Implement actual logic
    return FALSE;
}

boolean get_next_player_information(struct bungie_net_player_datum *player) {
    // TODO: Implement actual logic
    return FALSE;
}


boolean create_user_database(
	void)
{
	// Replaced with SQLite3 logic
	return open_users_db();
}

// Removed duplicate initialize_user_database definition to fix redefinition error.

static int last_response;
static void refresh_response_list(
	void)
{
	unsigned long i;
	last_response = 0;

	for (i = 0; i < MAXIMUM_PLAYER_SEARCH_RESPONSES; i++)
	{
		response_list[i].match_score = 0;
	}
}

static void add_player_to_response_list(
	struct bungie_net_online_player_data * opd)
{
	if (last_response<MAXIMUM_PLAYER_SEARCH_RESPONSES)
	{
		memcpy(&response_list[last_response].aux_data, &opd->aux_data, sizeof(struct metaserver_player_aux_data));
		memcpy(&response_list[last_response].player_data, &opd->player_data, MAXIMUM_PACKED_PLAYER_DATA_LENGTH);
		response_list[last_response].match_score++;
		last_response++;
	}
}

static char * ci_pn_match(
	char * s0,
	char * s1)
{
	static char buf0[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	static char buf1[MAXIMUM_PLAYER_NAME_LENGTH + 1];

	//---assert(s0);
	if(!s0 || !s1)
		return NULL;

	strcpy(buf0, s0);
	strcpy(buf1, s1);

	strlwr(buf0);
	strlwr(buf1);

	return (char *)strstr(buf0, buf1);
};

boolean set_user_game_data(
	unsigned long user_index,
	short game_type,
	struct player_stats * stats)
{
	struct bungie_net_player_datum player;
	boolean success= FALSE;

	if (get_player_information(NULL, user_index, &player))
	{
		player.ranked_score.damage_inflicted	= stats->points_killed;
		player.ranked_score.damage_received		= player.ranked_score.damage_received;
		player.ranked_score.games_played		= stats->games_played;
		player.ranked_score.wins				= stats->first_place_wins;
		player.ranked_score.losses				= stats->last_place_wins;
		player.room_id							= stats->default_room;
		player.ranked_score.points				= stats->score;
		
		if (update_player_information(NULL, user_index, online_player_data[user_index - 1].logged_in_flag, &player))
		{
			success = TRUE;
		}
	}

	return success;
}	

boolean get_user_game_data(
	unsigned long player_id,
	short game_index,
	struct player_stats * stats,
	word * flags)
{
	struct bungie_net_player_datum player;
	boolean success = FALSE;
	
	if (get_player_information(NULL, player_id, &player))
	{
		stats->points_killed					= player.ranked_score.damage_inflicted;
		stats->points_lost						= player.ranked_score.damage_received;
		stats->units_killed						= 0;
		stats->units_lost						= 0;
		stats->updates_since_last_game_played	= 0;
		stats->games_played						= player.ranked_score.games_played;
		stats->first_place_wins					= player.ranked_score.wins;
		stats->last_place_wins					= player.ranked_score.losses;
		stats->caste							= player.ranked_score.rank;
		stats->default_room						= player.room_id;
		stats->score							= player.ranked_score.points;
		success = TRUE;
	}


	return success;
}

boolean get_user_name(
	long user_index, 
	char * name)
{
	boolean success = FALSE;

	if (user_index <= total_players)
	{
		strcpy(name, online_player_data[user_index - 1].name);
		success = TRUE;
	}

	return success;
}

boolean set_myth_user_data(
	unsigned long user_index,
	char * buffer,
	short length)
{
	boolean success = FALSE;
	char * p;
	struct bungie_net_player_datum player;

	if (get_player_information(NULL, user_index, &player))
	{
		p = buffer;
		player.icon_index = (short)*p;
		p++; p++;
		p += sizeof(short);
		player.primary_color.red = SWAP2(*(word *)p);
		p += sizeof(word);
		player.primary_color.green = SWAP2(*(word *)p);
		p += sizeof(word);
		player.primary_color.blue = SWAP2(*(word *)p);
		p += sizeof(word);
		player.primary_color.flags = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.red = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.green = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.blue = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.flags = SWAP2(*(word *)p);
		p += sizeof(word);
		p++; p++;

		// _MYTHDEV Begin: order colors are unused, store new build version in order_pri_color.red
		player.aux_data.build_version = SWAP2(*(word *)p);
		// _MYTHDEV End

		p += sizeof(struct rgb_color); p += sizeof(struct rgb_color);
		strcpy(player.name, p);
		p += strlen(player.name) + 1;
		strcpy(player.team_name, p);
		p += strlen(player.team_name) + 1;

		if((p - buffer) != length)
			return FALSE;

		if (update_player_information(NULL, user_index, online_player_data[user_index - 1].logged_in_flag, &player))
		{
			success = TRUE;
		}
	}

	return success;
}

boolean get_myth_user_data(
	unsigned long user_index,
	char *buffer,
	short *length)
{
	boolean success = FALSE;

	if (user_index <= total_players)
	{
		memcpy(buffer, online_player_data[user_index - 1].player_data, MAXIMUM_PACKED_PLAYER_DATA_LENGTH);
		*length = online_player_data[user_index - 1].aux_data.player_data_length;
		success = TRUE;
	}

	return success;
}

void * build_rank_list(
	short game_type, 
	short maximum_users, 
	short *actual_users)
{
	*actual_users = get_user_count();

	return NULL;
}

boolean set_user_as_bungie_admin(
	long user_index, 
	boolean set)
{
	return FALSE;
}

