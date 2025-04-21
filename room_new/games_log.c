/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"

#include <stdio.h>

#define GAMES_LOG_FILE		"games_log"

struct
{
	FILE *games_log;
}
 games_log_globals;

boolean initialize_games_log(
	void)
{
	boolean success= FALSE;

	games_log_globals.games_log= fopen(GAMES_LOG_FILE, "a+");
	if (games_log_globals.games_log) success= TRUE;

	return success;
}

void shutdown_games_log(
	void)
{
	fclose(games_log_globals.games_log);
	games_log_globals.games_log= NULL;
}

void output_games_log_message(
	char *message,
	short room_id,
	unsigned long game_id)
{
  return;
}



