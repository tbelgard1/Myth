/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef	__GAMES_LOG_H
#define	__GAMES_LOG_H

boolean initialize_games_log(void);
void shutdown_games_log(void);
void output_games_log_message(char *message, short room_id, unsigned long game_id);

#endif
