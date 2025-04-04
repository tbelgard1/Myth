/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __GAME_SEARCH_PACKETS_H
#define __GAME_SEARCH_PACKETS_H

#include <byte_swapping.h>
#include <cseries.h>
#include <metaserver_common_structs.h>

#ifdef BN2_FULLVERSION
#define DEFAULT_GAME_SEARCH_PORT	7980
#elif defined(BN2_DEMOVERSION)
#define DEFAULT_GAME_SEARCH_PORT	7981
#endif

enum
{
	_net_game_in_progress_bit = 0,
	_net_game_has_password_bit,
	_net_game_invalid_bit,
	_net_game_is_tournament_round_bit,
	_net_game_is_closed_bit,
	NUMBER_OF_NET_BITS,

	_net_game_in_progress_flag = FLAG(_net_game_in_progress_bit),
	_net_game_has_password_flag = FLAG(_net_game_has_password_bit),
	_net_game_invalid_flag = FLAG(_net_game_invalid_bit),
	_net_game_is_tournament_round_flag = FLAG(_net_game_is_tournament_round_bit),
	_net_game_is_closed_flag = FLAG(_net_game_is_closed_bit)
};

enum
{
	_game_option_random_endgame_countdown_bit,
	_game_option_allow_multiplayer_teams_bit,
	_game_option_limited_visibility_bit,
	_game_option_no_ingame_rankings_bit,
	_game_option_allow_unit_trading_bit,
	_game_option_allow_veterans_bit,
	_game_option_continuation_bit,
	_game_option_cooperative_bit,
	_game_option_random_teams_bit,
	_game_option_limited_terrain_visibility_bit,
	_game_option_camera_tracking_initially_on_bit,
	_game_option_paused_bit,
	_game_option_local_time_paused_bit,
	_game_option_allow_alliances_bit,
	_game_option_allow_overhead_map_bit,
	NUMBER_OF_GAME_OPTION_BIT,

	_game_option_random_endgame_countdown_flag = FLAG(_game_option_random_endgame_countdown_bit),
	_game_option_allow_multiplayer_teams_flag = FLAG(_game_option_allow_multiplayer_teams_bit),
	_game_option_limited_visibility_flag = FLAG(_game_option_limited_visibility_bit),
	_game_option_no_ingame_rankings_flag = FLAG(_game_option_no_ingame_rankings_bit),
	_game_option_allow_unit_trading_flag = FLAG(_game_option_allow_unit_trading_bit),
	_game_option_allow_veterans_flag = FLAG(_game_option_allow_veterans_bit),
	_game_option_continuation_flag = FLAG(_game_option_continuation_bit),
	_game_option_cooperative_flag = FLAG(_game_option_cooperative_bit),
	_game_option_random_teams_flag = FLAG(_game_option_random_teams_bit),
	_game_option_limited_terrain_visibility_flag = FLAG(_game_option_limited_terrain_visibility_bit),
	_game_option_camera_tracking_initally_on_flag = FLAG(_game_option_camera_tracking_initially_on_bit),
	_game_option_paused_flag = FLAG(_game_option_paused_bit),
	_game_option_local_time_paused_flag = FLAG(_game_option_local_time_paused_bit),
	_game_option_allow_alliances_flag = FLAG(_game_option_allow_alliances_bit),
	_game_option_allow_overhead_map_flag = FLAG(_game_option_allow_overhead_map_bit)	
};

enum
{
	_body_count = 0,
	_steal_the_bacon,
	_last_man_on_the_hill,
	_scavenger_hunt,
	_flag_rally,
	_capture_the_flag,
	_balls_on_parade,
	_territories,
	_captures,
	_king_of_the_hill,
	_rugby
};

enum
{
	_gs_login_packet,
	_gs_update_packet,
	_gs_query_packet,
	_gs_query_response_packet,
	NUMBER_OF_GS_PACKETS
};

enum
{
	_add_new_game,
	_change_game_info,
	_remove_game
};

struct gs_login_packet
{
	long room_id;
};

struct gs_update_packet
{
	int type_of_update;
	long room_id;
	boolean game_is_ranked;
	struct metaserver_game_aux_data	aux_data;
	int game_data_length;
	struct metaserver_game_description game;
};

struct gs_query_packet
{
	unsigned long player_id;
	short game_type;
	short game_scoring;
	short unit_trading;
	short veterans;
	short teams;
	short alliances;
	short enemy_visibility;
	word pad;
};

struct gs_query_response_header
{
	// contains the number of responses
	// and the player to whom the responses are directed
	int number_of_responses;
	unsigned long player_id;
};

struct gs_query_response_segment
{
	int segment_length;
	long room_id;
	boolean	game_is_ranked;
	struct metaserver_game_aux_data	aux_data;
	int game_data_length;
	struct metaserver_game_description game;
};

struct g_bs_data
{
	byte_swap_code * bs_code;
	short length;
};

short build_gs_login_packet(char *buffer, long room_id);
short build_gs_update_packet(char *buffer, int type, long room_id, boolean game_is_ranked, 
	struct metaserver_game_aux_data *aux_data, int game_data_length, 
	struct metaserver_game_description *game);
short build_gs_query_packet(char *buffer, unsigned long player_id, char *game_name, char *map_name, 
	short game_scoring, short unit_trading, short veterans, short teams, 
	short alliances, short enemy_visibility);
int build_gs_query_response_packet(char *buffer, unsigned long user_id, struct query_response *qr);
short build_gs_update_delete_packet(char *buffer, int type, long game_id, long room_id, long creating_player_id);
boolean send_game_search_packet(int socket, char *buffer, short length);

boolean byte_swap_game_search_packet(char *buffer, boolean outgoing);
void byte_swap_game_data(char *buffer);

#endif
