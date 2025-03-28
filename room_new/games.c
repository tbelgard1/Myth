/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include "environment.h"
#include "stats.h"
#include "authentication.h"
#include "metaserver_common_structs.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "metaserver_codes.h"
#include "rank.h"
#include "games.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "server_code.h"
#include "room_globals.h"

static char error[1024];

static struct game_data * first_game = NULL;
static unsigned long room_global_game_id = 0;

unsigned long find_game_by_creator_id(
	unsigned creator_player_id)
{
	struct game_data * game;
	unsigned long game_id = 0;

	game = first_game;
	while (game)
	{
		if (game->creator_player_id == creator_player_id)
		{
			game_id = game->game_id;
			break;
		}
		game = game->next;
	}

	return game_id;
}

unsigned long create_game(
	unsigned long creator_player_id,
	short game_classification,
	short game_data_length,
	void * game_data)
{
	struct game_data * game;
	unsigned long game_id = 0;

	if(!game_data)
		return 0;
	if(game_data_length < 0 && game_data_length > MAXIMUM_METASERVER_GAME_DATA_SIZE)
		return 0;
	
	game = (struct game_data *)malloc(sizeof(struct game_data));
	if(game)
	{
		memset(game, 0, sizeof(struct game_data));
		game->game_id = ++room_global_game_id;
		game->creator_player_id = creator_player_id;
		game->ticks_at_creation = machine_tick_count();
		game->game_classification = game_classification;
		game->game_data_length = game_data_length;
		memcpy(game->game_data, game_data, game_data_length);
		game_id = game->game_id;
		game->reported= FALSE;
		if (first_game)
		{
			struct game_data * previous = first_game;

			while (previous->next != NULL) previous = previous->next;
			previous->next = game;
		}
		else
		{
			first_game = game;
		}
	}

	return game_id;
}

void change_game_information(
	unsigned long game_id,
	short game_classification,
	short game_data_length,
	void * game_data)
{
	struct game_data * game;

	if(!game_data)
		return;
	if(game_data_length < 0 && game_data_length > MAXIMUM_METASERVER_GAME_DATA_SIZE)
		return;

	game = find_game_by_id(game_id);
	if (game)
	{
		memcpy(game->game_data, game_data, game_data_length);
		game->game_classification = game_classification;
	}
}

void add_player_to_game(
	unsigned long game_id,
	unsigned long new_player_id)
{
	struct game_data * game;

	game = find_game_by_id(game_id);
	if (game)
	{
		short index;
		boolean player_found = FALSE;

		for (index = 0; index < game->player_count; ++index)
		{
			if (game->players[index].player_id == new_player_id)
			{
				player_found = TRUE;
				break;
			}
		}

		if (!player_found)
		{
			game->players[game->player_count].player_id = new_player_id;
			game->players[game->player_count].reported_standings = FALSE;
			game->player_count++;
		}
	}
}

void delete_game(
	unsigned long game_id)
{
	struct game_data * game;

	if(game_id > room_global_game_id)
		return;

	output_games_log_message("delete game called", room_globals.room_identifier, game_id);

	game = find_game_by_id(game_id);
	if (game) game->ticks_at_game_deletion = machine_tick_count();
}

boolean report_standings_for_game(
	unsigned long game_id,
	unsigned long player_id,
	struct bungie_net_game_standings * standings)
{
	struct game_data * game;
	boolean success = FALSE;

	game = find_game_by_id(game_id);
	if (game)
	{
		short index;

		for (index = 0; index < game->player_count; ++index)
		{
			if (game->players[index].player_id == player_id)
			{
				if (!game->players[index].reported_standings)
				{
					sprintf(error, "player %u has reported standings", player_id);
					output_games_log_message(error, room_globals.room_identifier, game_id);
					game->players[index].standings = *standings;
					game->players[index].reported_standings = TRUE;
					success = TRUE;
					break;
				}
			}
		}

		for (index = 0; index < game->player_count; ++index)
		{
			if (!game->players[index].reported_standings)
			{
				break;
			}
		}

		if (index==game->player_count)
			report_game_to_userd(game);
	}

	return success;
}

void reset_game_for_new_round(
	unsigned long game_id)
{
	struct game_data * game;

	game = find_game_by_id(game_id);
	if (game)
	{
		short index;

		for (index = 0; index < MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME; ++index)
		{
			if (game->players[index].reported_standings == TRUE)
			{
				report_game_to_userd(find_game_by_id(game_id));
				break;
			}
		}

		for (index = 0; index < MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME; ++index)
		{
			game->players[index].player_id = 0;
			game->players[index].reported_standings = FALSE;
		}

		game->player_count = 0;
		game->ticks_at_creation = time(NULL);
	}
	else
	{
		sprintf(error, "could not find game id %u in room?", game_id);
		output_games_log_message(error, room_globals.room_identifier, game_id);
	}
}

void reset_player_list(
	unsigned long game_id)
{
	struct game_data * game;

	game = find_game_by_id(game_id);
	if (game)
	{
		short index;

		for (index = 0; index < MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME; ++index)
		{
			game->players[index].player_id = 0;
			game->players[index].reported_standings = FALSE;
		}

		game->player_count = 0;
	}
}

void idle_games(
	void)
{
	struct game_data * game;
	boolean game_ended_normally = FALSE;
	boolean score_game = FALSE;

	game = first_game;
	while (game)
	{
		short index;

		if (game->ticks_at_game_deletion)
		{
			score_game = TRUE;
			for (index = 0; index < game->player_count; index++)
			{
				if (!game->players[index].reported_standings)
				{
					score_game = FALSE;
					break;
				}
			}

			if ((machine_tick_count() - game->ticks_at_game_deletion) > 
				TICKS_BEFORE_SCORING_ABNORMALLY_ENDED_GAME)
			{
				score_game = TRUE;
			}
			else
			{
				for (index = 0; index < game->player_count; index++)
				{
					if (game->players[index].standings.game_ended_code == _game_ended_normally)
					{
						if ((machine_tick_count() - game->ticks_at_game_deletion) > 
							TICKS_BEFORE_SCORING_NORMALLY_ENDED_GAME)
						{
							score_game = TRUE;
							break;
						}
					}
				}
			}

			if (score_game)
			{
				struct game_data * previous;

				if(!first_game)
					return;
				if (game == first_game)
				{
					first_game = first_game->next;
				}
				else
				{
					previous = first_game;
					while (previous->next)
					{
						if (previous->next == game)
						{
							previous->next = game->next;
							break;
						}
						previous = previous->next;
					}
				}

				report_game_to_userd(game);
				free(game);
				break;
			}
		}
		game = game->next;
	}
}

struct game_data * find_game_by_id(
	unsigned long game_id)
{
	struct game_data * game;

	game = first_game;
	while (game)
	{
		if (game->game_id == game_id) break;
		game = game->next;
	}

	return game;
}




