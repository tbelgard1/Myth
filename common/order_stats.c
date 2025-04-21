/*
 order_stats.c
*/

#include "cseries.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "order_stats.h"

void add_order_stats(
	struct order_stats *a, 
	struct order_stats *b, 
	struct order_stats *result)
{
	struct order_stats temp;

	memset(&temp, 0, sizeof(struct order_stats));
	temp.games_played = a->games_played + b->games_played;
	temp.points_killed = a->points_killed + b->points_killed;
	temp.points_lost = a->points_lost + b->points_lost;
	temp.updates_since_last_game_played = a->updates_since_last_game_played;
	temp.score = a->score + b->score;
	temp.first_place_wins = a->first_place_wins + b->first_place_wins;
	temp.last_place_wins = a->last_place_wins + b->last_place_wins;

	// don't let anyone go negative
	if (temp.score < 0) temp.score = 0;

	// theese are added, since the delta will have a zero..
	temp.caste = a->caste + b->caste;
	temp.default_room = a->default_room + b->default_room;
	
	memcpy(result, &temp, sizeof(struct order_stats));

	return;
}

// calculate a score based on the comparison of a and b
/*
	S= (((1st-last)/#games)*A + (points_killed*B)/points_lost + (games_Played*C))*(2g/d)
*/

long convert_order_stats_to_score( // this is the magic function
	struct order_stats * a)
{
	return a->score;
}

boolean empty_order_stats(
	struct order_stats * a)
{
	struct order_stats empty;
	boolean no_stats = FALSE;
	
	memset(&empty, 0, sizeof(struct order_stats));
	if(memcmp(a, &empty, sizeof(struct order_stats))==0)
		no_stats= TRUE;

	return no_stats;
}

// calculate delta if a beats b
long calculate_order_delta_score(
	long friendly, 
	long enemy, 
	boolean win)
{
	long delta_score;
	float wins_expected, power;
	
	power = (float) (enemy - friendly) / G;
	wins_expected = 1.0/(1.0 + pow(10.0, power));
	delta_score = ((win ? 1.0 : 0.0) - wins_expected) * K;

	return delta_score;
}
