/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

struct gamedata
{
	long room_id;
	unsigned long game_id;

	struct metaserver_game_aux_data aux_data;
	struct metaserver_game_description description;

	unsigned long flags;

	char game_name[NETWORK_GAME_NAME_LENGTH+1];
	char map_name[NETWORK_MAP_NAME_LENGTH+1];
};

struct games_list_query
{
	unsigned long flags;

	short game_scoring;
	short unit_trading;
	short veterans;
	short teams;
	short alliances;
	short enemy_visibility;

	char game_name[NETWORK_GAME_NAME_LENGTH+1];
	char map_name[NETWORK_MAP_NAME_LENGTH+1];
};

void games_list_initialize(void);
void games_list_dispose(void);

boolean games_list_add_entry(struct gamedata *gamedata);
void games_list_remove_entry(long room_id, struct metaserver_game_aux_data *aux_data);

struct gamedata *games_list_search_for_first_match(struct games_list_query *query);
struct gamedata *games_list_search_for_next_match(void);
