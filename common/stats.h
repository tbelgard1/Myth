#ifndef __STATS_H__
#define __STATS_H__

/*
	dScore= (Wactual-Wexpected)*K;
	Wexpected= 1/(1+10^((opponentscore-playerscore)/G))
*/

#define MYTH	0

struct player_stats { // 32 bytes
	long score;

	long points_killed;
	long points_lost;
	long units_killed;
	long units_lost;
	
	short updates_since_last_game_played;
	short games_played;

	short first_place_wins;
	short last_place_wins;				// host drops

	short caste;
	short default_room;
};

void add_player_stats(struct player_stats *a, struct player_stats *b, struct player_stats *result);

long convert_player_stats_to_score(struct player_stats *a);
boolean empty_stats(struct player_stats *a);

void print_stats(struct player_stats *a);
#endif
