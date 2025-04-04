/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __BUNGIE_NET_PLAYER__
#define __BUNGIE_NET_PLAYER__

#include "metaserver_common_structs.h"

enum
{
	TAG_FILE_NAME_LENGTH = 8,
	MAXIMUM_LOGIN_LENGTH = 15,
	MAXIMUM_PASSWORD_LENGTH = 15,
	MAXIMUM_PLAYER_NAME_LENGTH = 31,
	MAXIMUM_DESCRIPTION_LENGTH = 431,
	MAXIMUM_NUMBER_OF_GAME_TYPES = 16,
	MAXIMUM_BUDDIES = 8,
	MAXIMUM_ORDER_MEMBERS = 16, // kept for restricting new additions
	STEFANS_MAXIMUM_ORDER_MEMBERS = 32, // added to handle some orders that have over 20 ppl from the olden days (my buddies in OMAG, civil & CP)
	MAXIMUM_PACKED_PLAYER_DATA_LENGTH = 128,
	NUMBER_OF_TRACKED_OPPONENTS = 10
};

struct rgb_color
{
	word red, green, blue;	
	word flags;
};

enum
{
	INACTIVE,
	UNACKNOWLEDGED,
	ACTIVE,
	OFFLINE
};

struct order_member
{
	unsigned long player_id;
	boolean online;
};

struct buddy_entry
{
	unsigned long player_id;
	char active;
	char c[3];
};

struct bungie_net_player_score_datum
{
	short games_played;
	short wins, losses, ties;
	long damage_inflicted, damage_received;

	short disconnects;
	word pad;

	short points;
	short rank;

	short highest_points;
	short highest_rank;

	unsigned long numerical_rank;

	char unused[16];
};

enum
{
        _bungie_employee_flag = 0x01,
        _account_is_kiosk_flag = 0x02
};

struct bungie_net_player_datum
{
	unsigned long player_id;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	char password[MAXIMUM_PASSWORD_LENGTH + 1];
	
	boolean administrator_flag;
	unsigned int special_flags;
	boolean player_is_banned_flag;
	boolean unused_flags[5];

	long last_login_ip_address;

	long last_login_time;
	long last_game_time;
	long last_ranked_game_time;

	long room_id;

	struct buddy_entry buddies[MAXIMUM_BUDDIES];

	short order_index;
	short icon_index;
	short icon_collection_name[TAG_FILE_NAME_LENGTH + 1];

	char name[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	char team_name[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	struct rgb_color primary_color, secondary_color;

	char description[MAXIMUM_DESCRIPTION_LENGTH + 1];

	long banned_time;
	long ban_duration;
	long times_banned;

	short country_code;		

	char unused[42];

	unsigned long last_opponent_index;
	unsigned long last_opponents[NUMBER_OF_TRACKED_OPPONENTS];
	
	struct bungie_net_player_score_datum unranked_score;
	struct bungie_net_player_score_datum ranked_score;
	struct bungie_net_player_score_datum ranked_scores_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];
};

struct bungie_net_online_player_data
{
	unsigned long online_data_index;
	unsigned long player_id;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	char name[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	long room_id;
	short order;
	boolean logged_in_flag;
	struct metaserver_player_aux_data aux_data;
	char player_data[MAXIMUM_PACKED_PLAYER_DATA_LENGTH];
	long fpos;
};

#endif // __BUNGIE_NET_PLAYER__
