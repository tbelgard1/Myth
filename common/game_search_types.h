/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __GAME_SEARCH_TYPES_H
#define __GAME_SEARCH_TYPES_H

struct new_game_parameter_data
{
	short				type;
	short				scoring;
	unsigned long		option_flags;
	long				time_limit;
	unsigned long		scenario_tag;
	short				difficulty_level;
	short				maximum_players;
	word				initial_team_random_sead;
	short				maximum_teams;
	unsigned long		random_seed;
	long				pregame_time_limit;
	long				unused[2];
	short				unused_short;
	short				plugin_count;
	char				plugin_data[512];
};

struct metaserver_game_description
{
	struct new_game_parameter_data	parameters;
	long							public_tags_checksum;
	long							private_tags_checksum;
	word							flags;
	char							player_count;
};

#endif
