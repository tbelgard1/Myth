/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __GAMES_H
#define __GAMES_H

#define	TICKS_BEFORE_SCORING_ABNORMALLY_ENDED_GAME		7200
#define	TICKS_BEFORE_SCORING_NORMALLY_ENDED_GAME		120

enum
{
	_unranked_normal,
	_unranked_order,
	_ranked_normal,
	_ranked_order,
	_tournament_normal,
	_tournament_order,
	NUMBER_OF_GAME_CLASSIFICATIONS
};

struct player_game_data
{
	unsigned long player_id;

	boolean reported_standings;
	struct bungie_net_game_standings standings;
};

struct game_data
{
	unsigned long game_id;
	unsigned long creator_player_id;
	long ticks_at_creation;
	long ticks_at_game_deletion;

	short game_classification;
	short reported;

	struct player_game_data players[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	short player_count;

	short game_data_length;
	char game_data[MAXIMUM_METASERVER_GAME_DATA_SIZE];

	struct game_data * next;
};

struct game_data *find_game_by_id(
	unsigned long game_id);

unsigned long find_game_by_creator_id(
	unsigned creator_player_id);

unsigned long create_game(
	unsigned long creator_player_id,
	short game_classification,
	short game_data_length,
	void * game_data);

void change_game_information(
	unsigned long game_id,
	short game_classification,
	short game_data_length,
	void * game_data);

void add_player_to_game(
	unsigned long game_id,
	unsigned long new_player_id);

void delete_game(
	unsigned long game_id);

boolean report_standings_for_game(
	unsigned long game_id,
	unsigned long player_id,
	struct bungie_net_game_standings * standings);

void reset_game_for_new_round(
	unsigned long game_id);

void reset_player_list(
	unsigned long game_id);

void idle_games(
	void);

#endif
