/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include "metaserver_common_structs.h"
#include "sl_list.h"
#include "games_list.h"

#include "game_search_packets.h"

static struct sl_list *games_list;

static struct
{
	struct sl_list_element *search_element;
	struct games_list_query query;
}
games_list_globals;

static int games_list_comp_func(void *k0, void *k1);

void games_list_initialize(
	void)
{
	games_list= sl_list_new("games list", games_list_comp_func);
	if (!games_list) vhalt("could not initialize game search list, system memory unavailable?");
}

void games_list_dispose(
	void)
{
	sl_list_dispose(games_list);
}

boolean games_list_add_entry(
	struct gamedata *gamedata)
{
	struct sl_list_element *element;
	struct gamedata *data;
	boolean success= FALSE;

	element= sl_list_search_for_element(games_list, gamedata);
	if (element)
	{
		void *data= element->data;

		sl_list_remove_element(games_list, element);
		sl_list_dispose_element(games_list, element);
		free(data);
	}

	data= malloc(sizeof(struct gamedata));
	if (data)
	{
		memcpy(data, gamedata, sizeof(struct gamedata));
		element= sl_list_new_element(games_list, data, data);
		if (element)
		{
			sl_list_insert_element(games_list, element);
			success= TRUE;
		}
	}

	return success;
}

void games_list_remove_entry(
	long room_id, 
	struct metaserver_game_aux_data *aux_data)
{
	struct sl_list_element *element;
	struct gamedata data;

	assert(aux_data);

	memset(&data, 0, sizeof(struct gamedata));
	data.room_id= room_id;
	data.game_id= aux_data->game_id;
	data.aux_data= *aux_data;

	element= sl_list_search_for_element(games_list, &data);
	if (element) sl_list_remove_element(games_list, element);
}

struct gamedata *games_list_search_for_first_match(
	struct games_list_query *query)
{
	struct sl_list_element *element;
	struct gamedata *ret= NULL;

	element= sl_list_get_head_element(games_list);
	if (element) 
	{
		do
		{
			struct gamedata *game;

			game= (struct gamedata *)element->data;
			if (game && query->game_name[0])
			{
				if (strstr(game->game_name, query->game_name))
				{
					// matches
				}
				else
				{
					game= NULL;
				}
			}

			if (game && query->map_name[0])
			{
				if (strstr(game->map_name, query->map_name))
				{
					// matches
				}
				else
				{
					game= NULL;
				}
			}

			if (game && query->game_scoring!=NONE)
			{
				if (query->game_scoring==game->description.parameters.scoring)
				{
					// matches
				}
				else
				{
					game= NULL;
				}
			}

			if (game)
			{
				switch (query->unit_trading)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_unit_trading_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_unit_trading_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (query->veterans)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_veterans_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_veterans_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (query->teams)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_multiplayer_teams_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_multiplayer_teams_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (query->alliances)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_alliances_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_alliances_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (query->enemy_visibility)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_overhead_map_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_overhead_map_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				ret= game;
				break;
			}

		} while (element= (sl_list_get_next_element(games_list, element)));

		games_list_globals.search_element= element;
		games_list_globals.query= *query;
	}

	return ret;
}

struct gamedata *games_list_search_for_next_match(
	void)
{
	struct sl_list_element *element;
	struct gamedata *ret= NULL;

	element= sl_list_get_next_element(games_list, games_list_globals.search_element);
	if (element) 
	{
		do
		{
			struct gamedata *game;

			game= (struct gamedata *)element->data;
			if (game && games_list_globals.query.game_name[0])
			{
				if (strcasecmp(game->game_name, games_list_globals.query.game_name)==0)
				{
					// matches
				}
				else
				{
					game= NULL;
				}
			}

			if (game && games_list_globals.query.map_name[0])
			{
				if (strcmp(game->map_name, games_list_globals.query.map_name)==0)
				{
					// matches
				}
				else
				{
					game= NULL;
				}
			}

			if (game && games_list_globals.query.game_scoring!=NONE)
			{
				if (games_list_globals.query.game_scoring==game->description.parameters.scoring)
				{
					// matches
				}
				else
				{
					game= NULL;
				}
			}

			if (game)
			{
				switch (games_list_globals.query.unit_trading)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_unit_trading_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_unit_trading_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (games_list_globals.query.veterans)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_veterans_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_veterans_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (games_list_globals.query.teams)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_multiplayer_teams_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_multiplayer_teams_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (games_list_globals.query.alliances)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_alliances_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_alliances_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				switch (games_list_globals.query.enemy_visibility)
				{
				case _required:
					if (!(game->description.parameters.option_flags&_game_option_allow_overhead_map_flag)) game= NULL;
					break;
				
				case _despised:
					if (game->description.parameters.option_flags&_game_option_allow_overhead_map_flag) game= NULL;
					break;

				case _dontcare:
					break;

				default:
					halt();
				}
			}

			if (game)
			{
				ret= game;
				break;
			}	
			
		} while (element= (sl_list_get_next_element(games_list, element)));

		games_list_globals.search_element= element;
	}

	return ret;
}

static int games_list_comp_func(
	void *k0, 
	void *k1)
{
	struct gamedata *g0, *g1;
	int ret= -1;

	g0= (struct gamedata *)k0; g1= (struct gamedata *)k1;
	if (g0->aux_data.creating_player_id==g1->aux_data.creating_player_id) ret= 0;

	return ret;
}
