/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef	__RANK_H
#define	__RANK_H

#include "bungie_net_order.h"


//

enum
{
	_dagger,
	_dagger_with_hilt,
	_kris_knife,
	_sword_and_dagger,
	_crossed_swords,
	_crossed_axes,
	_shield,
	_shield_crossed_swords,
	_shield_crossed_axes,
	_simple_crown,
	_crown,
	_nice_crown,
	_eclipsed_moon,
	_moon,
	_eclipsed_sun,
	_sun,
	_comet,
};

enum 
{
	GAMES_PLAYED_DAGGER_CASTE= 1,
	GAMES_PLAYED_DAGGER_WITH_HILT_CASTE= 2,
	GAMES_PLAYED_KRIS_DAGGER_CASTE= 3,
	ECLIPSED_MOON_PLAYER_COUNT= 3,
	MOON_PLAYER_COUNT= 2,
	ECLIPSED_SUN_PLAYER_COUNT= 1,
	SUN_PLAYER_COUNT= 1,
	COMET_PLAYER_COUNT= 1,
	TOTAL_NAMED_PLAYER_COUNT= 8,
	NUMBER_OF_RANKED_GAME_TYPES = 8,
	NUMBER_OF_NORMAL_CASTES = 12
};

struct bungie_net_player_stats	// 1548
{
	boolean administrator_flag;	// 4
	boolean bungie_employee_flag;	// 4
	short order_index;		// 2
	short icon_index;		// 2
	struct rgb_color primary_color;	// 8
	struct rgb_color secondary_color;	// 8
	struct bungie_net_player_score_datum unranked_score_datum;	// 28
	struct bungie_net_player_score_datum ranked_score_datum ;	// 28
	struct bungie_net_player_score_datum ranked_score_datum_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];	// 448
	struct bungie_net_player_score_datum order_unranked_score_datum;									// 28
	struct bungie_net_player_score_datum order_ranked_score_datum;										// 28
	struct bungie_net_player_score_datum order_ranked_score_datum_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];	// 448
	char login[MAXIMUM_LOGIN_LENGTH + 1];	// 16
	char name[MAXIMUM_PLAYER_NAME_LENGTH + 1];	// 32
	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];	// 32
	char description[MAXIMUM_DESCRIPTION_LENGTH + 1];	// 432
};

struct ranking_data
{
	unsigned long average;
	unsigned long best;
};

struct game_rank_data							// 72
{
	char top_ranked_player[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	struct ranking_data points;
	struct ranking_data games_played;
	struct ranking_data wins;
	struct ranking_data damage_inflicted;
	struct ranking_data damage_received;
};

#define	NUMBER_OF_GAME_RANK_DATAS_IN_OVERALL_RANKING_DATA	36	

struct overall_ranking_data					// 2600
{
	unsigned long total_users;
	unsigned long total_orders;
	struct game_rank_data unranked_game_data;
	struct game_rank_data ranked_game_data;
	struct game_rank_data ranked_game_data_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];
	struct game_rank_data order_unranked_game_data;
	struct game_rank_data order_ranked_game_data;
	struct game_rank_data order_ranked_game_data_by_game_type[MAXIMUM_NUMBER_OF_GAME_TYPES];
};

struct caste_breakpoint_data					// 80
{
	unsigned long normal_caste_breakpoints[NUMBER_OF_NORMAL_CASTES];
	unsigned long eclipsed_moon_player_ids[ECLIPSED_MOON_PLAYER_COUNT];
	unsigned long moon_player_ids[MOON_PLAYER_COUNT];
	unsigned long eclipsed_sun_player_ids[ECLIPSED_SUN_PLAYER_COUNT];
	unsigned long sun_player_ids[SUN_PLAYER_COUNT];
	unsigned long comet_player_ids[COMET_PLAYER_COUNT];
};

boolean build_overall_ranking_data(
	struct caste_breakpoint_data * caste_breakpoints,
	struct overall_ranking_data * overall_rank_data);

#endif
