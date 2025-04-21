/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __ROOM_GLOBALS__
#define __ROOM_GLOBALS__

#include "room_packets.h"

struct global_room_data
{
	boolean logged_into_userd;
	boolean	logged_into_game_search_engine;
  	boolean logged_into_player_search_engine;

	time_t last_userd_ping;

	unsigned long flags;
	
	int userd_socket; // may be NONE if not logged in...
	int game_search_socket; // may be NONE if not logged in...
	int player_search_socket;
	short room_port;
	short room_identifier;
	short game_type;
	short seconds_before_update;
	short maximum_players_per_room;
	short unused[2];
	boolean ranked_room;
	boolean tournament_room;
	struct caste_breakpoint_data caste_breakpoints;
	struct overall_ranking_data overall_rank_data;
	char url_for_version_update[ROOM_MAXIMUM_UPDATE_URL_SIZE];
	char motd[ROOM_MAXIMUM_MOTD_SIZE];
	
	time_t launch_time;
	boolean squelched;
};

extern struct global_room_data room_globals;

void change_room_motd(char *motd);

#endif // __ROOM_GLOBALS__
