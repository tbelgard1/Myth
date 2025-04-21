/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"

#ifndef __METASERVER_COMMON_STRUCTS_H__
#define __METASERVER_COMMON_STRUCTS_H__
enum {
	MAXIMUM_METASERVER_GAME_NAME= 31,
	MAXIMUM_METASERVER_GAME_DATA_SIZE= 1024,
	MAXIMUM_METASERVER_PLAYER_DATA_SIZE= 128,
	MAXIMUM_METASERVER_USERNAME= 31,
	MAXIMUM_METASERVER_ORDERNAME= 31,
	MAXIMUM_METASERVER_PASSWORD= 15,
	MOTD_CHANGE_LOGIN_KEY_SIZE=8,
	MOTD_QUERY_LOGIN_KEY_SIZE=8,
	MAXIMUM_MOTD_SIZE=127,
	MAXIMUM_USERS_PER_ROOM= 64,
	MAXIMUM_TEAMS_PER_MAP= 16,
	MAXIMUM_PLAYERS_PER_MAP= 16,
	MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME= 16
};

enum
{
	NETWORK_GAME_NAME_LENGTH= 31,
	NETWORK_MAP_NAME_LENGTH= 63
};

enum {
	_player_is_administrator_bit, 
	_player_is_anonymous_bit, 
	_player_is_bungie_caste_icon_bit, 
	NUMBER_OF_PLAYER_FLAG_BITS,
	
	_player_is_administrator_flag= FLAG(_player_is_administrator_bit),
	_player_is_anonymous_flag= FLAG(_player_is_anonymous_bit),
	_player_is_bungie_caste_icon_flag= FLAG(_player_is_bungie_caste_icon_bit)
};

enum {
	_add_player_verb,
	_delete_player_verb,
	_change_player_verb,
	NUMBER_OF_PLAYER_VERBS,
	
	_add_game_verb= 0,
	_delete_game_verb,
	_change_game_verb,
	NUMBER_OF_GAME_VERBS
};

struct metaserver_player_aux_data { // 16 bytes
	word verb;
	word flags;
	long ranking;
	unsigned long player_id;
	long room_id;
	short caste;
	short player_data_length;
	short order;
	short pad;
};

enum {
	_data_is_last_chunk_bit,
	NUMBER_OF_DATA_CHUNK_BITS,
	
	_data_is_last_chunk_flag= FLAG(_data_is_last_chunk_bit)
};

struct data_chunk_identifier_data {
	long flags;
	long type;
	long offset;
	long length;
};

enum
{
	_unranked_room,
	_ranked_room,
	_tournament_room
};

struct room_info {
	short room_id;
	short player_count;
	long host;
	short port;
	short game_count;
	short room_type;
	short unused[5];
};

/* for scoring games.. */
struct player_list_packet_entry {
	unsigned long player_id;
};

struct score_list_packet_entry {
	unsigned long player_id;
	short place;
	short kills;
	short casualties;
	short points_killed;
	short points_lost;
	short unused[3];
 };

struct metaserver_game_aux_data { // 32 bytes
	long game_id;
	long host;
	short port;
	char verb;
	char version;
	long seconds_remaining;
	long creating_player_id;
	short game_data_size;
	short unused_short;
	long unused[2];
};

struct new_game_parameter_data
{
	short type;
	short scoring;
	unsigned long option_flags;
	long time_limit;
	unsigned long scenario_tag;
	short difficulty_level;
	short maximum_players;
	word initial_team_random_sead;
	short maximum_teams;
	unsigned long random_seed;
	long pregame_time_limit;
	long unused[2];
	short unused_short;
	short plugin_count;
	char plugin_data[512];
};

struct game_search_query {
	short game_type;
	short game_scoring;
	short veterans;
	short unit_trading;
	short teams;
	short alliances;
	short enemy_visibility;
	short unused;
	char game_name[NETWORK_GAME_NAME_LENGTH+1];
	char map_name[NETWORK_MAP_NAME_LENGTH+1];
};

struct metaserver_game_description
{
	struct new_game_parameter_data	parameters;
	long				public_tags_checksum;
	long				private_tags_checksum;
	word				flags;
	char				player_count;
};

struct room_login_successful_data {
	unsigned long user_id;
	short maximum_players_per_room;
	short unused;
};

struct successful_login_data { // data is this on _connection_established message
	short maximum_players_per_room;
	short unused_short;
	long unused[3];
};

enum
{
	_game_ended_normally,
	_game_ended_with_server_loss,
	_game_ended_with_player_quit,
	NUMBER_OF_GAME_ENDED_CODES
};

struct bungie_net_game_standings_team
{
	short place; // (zero is first place, one is second, etc.)
	
	short captain_player_index;
	short team_valid_flag; // FALSE if there were no players on this team and it should be ignored

	short team_eliminated_flag; // TRUE if all of this teams units were wiped out
	long time_of_elimination; // <=actual_time_elapsed (only valid if the team was eliminated)
};

struct bungie_net_game_standings_player
{
	short team_index; // irrelevant except it matches other players on the same team

	short player_finished_game_flag; // FALSE if this player was dropped (for any reason) before the game ended
	long time_played; // <=actual_time_elapsed (only valid if the player did not finish the game)

	long units_killed, units_lost;
	long points_killed, points_lost;

	long bungie_net_player_id;
};

struct bungie_net_game_standings
{
	short game_ended_code;

	short game_scoring;
	long version_number;

	long time_limit, actual_time_elapsed;

	long bungie_net_player_id; // the player_id reporting these standings

	short number_of_players; // who started the game
	short number_of_teams; // who started the game
	struct bungie_net_game_standings_team teams[MAXIMUM_TEAMS_PER_MAP];
	struct bungie_net_game_standings_player players[MAXIMUM_PLAYERS_PER_MAP];
};

enum
{
	_required= 0,
	_despised,
	_dontcare
};

#define	DATA_INDEX_UNUSED	0xAA55AA55
#define MAXIMUM_GAME_SEARCH_RESPONSES	5

struct query
{
	char game_name[NETWORK_GAME_NAME_LENGTH+1];
	char map_name[NETWORK_MAP_NAME_LENGTH+1];
	short game_type;
	short game_scoring;
	short unit_trading;
	short veterans;
	short alliances;
	short enemy_visibility;
};

struct query_response
{
	unsigned long data_index;
	unsigned long match_value;
	long room_id;
	boolean game_is_ranked;
	struct metaserver_game_aux_data aux_data;
	int game_data_length;
	struct metaserver_game_description game;
	char game_name[NETWORK_GAME_NAME_LENGTH+1];
	char map_name[NETWORK_MAP_NAME_LENGTH+1];
};

// patch flags.
enum {
	_required_patch_bit,
	_launch_after_download_patch_bit,
	_ask_before_download_bit,
	NUMBER_OF_METASERVER_PATCH_BITS,

	_required_patch_flag= FLAG(_required_patch_bit),
	_launch_after_download_patch_flag= FLAG(_launch_after_download_patch_bit),
	_ask_before_download_flag= FLAG(_ask_before_download_bit)
};

// version information
struct version_entry {
	long group; // tag group, or appl (shlb?)
	long subgroup; // myth, uber, mclt, etc, 01cb
	long version; // last modification date?
};

#endif
