/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __USERS_H__
#define __USERS_H__

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

#endif // __USERS_H__
