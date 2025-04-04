#ifndef __ORDER_STATS_H
#define __ORDER_STATS_H

// Elo rating system constants
#define G 400.0f  // Rating difference that gives 10:1 odds
#define K 32      // Maximum rating change per game

struct order_stats { 
	long score;

	long points_killed;
	long points_lost;
	
	short updates_since_last_game_played;
	short games_played;

	short first_place_wins;
	short last_place_wins;

	short caste;
	short default_room;
};

void add_order_stats(struct order_stats *a, struct order_stats *b, struct order_stats *result);

long convert_order_stats_to_score(struct order_stats *a);
boolean empty_order_stats(struct order_stats *a);

// calculate delta if a beats b
long calculate_order_delta_score(long friendly, long enemy, boolean won);

#endif // __ORDER_STATS_H
