/*
STATS.C
*/

#include "cseries.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "stats.h"

void add_player_stats(
	struct player_stats *a, 
	struct player_stats *b, 
	struct player_stats *result)
{
	struct player_stats temp;

	memset(&temp, 0, sizeof(struct player_stats));
	temp.games_played= a->games_played+b->games_played;
	temp.points_killed= a->points_killed+b->points_killed;
	temp.points_lost= a->points_lost+b->points_lost;
	temp.units_killed= a->units_killed+b->units_killed;
	temp.units_lost= a->units_lost+b->units_lost;
	temp.updates_since_last_game_played= a->updates_since_last_game_played;
	temp.score= a->score+b->score;
	temp.first_place_wins= a->first_place_wins+b->first_place_wins;
	temp.last_place_wins= a->last_place_wins+b->last_place_wins;

	// don't let anyone go negative
	if (temp.score<0) temp.score= 0;

	// these are added, since the delta will have a zero..
	temp.caste= a->caste+b->caste;
	temp.default_room= a->default_room+b->default_room;
	
	memcpy(result, &temp, sizeof(struct player_stats));

	return;
}

// calculate a score based on the comparison of a and b
/*
	S= (((1st-last)/#games)*A + (points_killed*B)/points_lost + (games_Played*C))*(2g/d)
*/

long convert_player_stats_to_score( // this is the magic function
	struct player_stats *a)
{
#if 0
	long score;
	short A, B, C, number_of_days_per_game;
	long days_online= (time(NULL)-a->time_at_initial_login)/(24*60*60);
	long wins, damage, played;
	long attenuation;
	long max_attenuation= SHORT_FIXED_ONE+(SHORT_FIXED_ONE/4);
		
	if(a->caste<=2) // dagger to sword
	{
		A= B= 0;
		C= SHORT_FIXED_ONE;
		number_of_days_per_game= 5*SHORT_FIXED_ONE;
	} 
	else if (a->caste<=5) // sword & dagger to crossed axes
	{
		A= 2*SHORT_FIXED_ONE;
		B= SHORT_FIXED_ONE;
		C= SHORT_FIXED_ONE;
		number_of_days_per_game= 4*SHORT_FIXED_ONE;
	}
	else if (a->caste<=11) // shield to imperial crown
	{
		A= 3*SHORT_FIXED_ONE;
		B= 2*SHORT_FIXED_ONE;
		C= 1*SHORT_FIXED_ONE;
		number_of_days_per_game= 3*SHORT_FIXED_ONE;
	}

	if(!days_online) days_online= 1;
	if(a->games_played)
	{
		wins= ((a->first_place_wins-a->last_place_wins)*A)/a->games_played;
		attenuation= (number_of_days_per_game*days_online)/a->games_played;
	} else {
		wins= 0;
		attenuation= SHORT_FIXED_ONE;
	}
	
	if(a->points_lost)
	{
		damage= ((a->points_killed)*B)/a->points_lost;
	} else {
		damage= 0; // ie you haven't really played yet.
	}
	played= (a->games_played*C);

	score= (wins+damage+played)/SHORT_FIXED_ONE;

	// attenuate score based on online time
	if(attenuation>max_attenuation) attenuation= max_attenuation;
	
	score= (score*attenuation)/SHORT_FIXED_ONE;
#endif
	return a->score;
}

void print_stats(
	struct player_stats *a)
{
	printf(" Played: %d Score: %ld\n", a->games_played, a->score);
	printf(" Points: Killed: %ld Lost: %ld\n", a->points_killed, a->points_lost);
	printf(" Units: Killed: %ld Lost: %ld\n", a->units_killed, a->units_lost);
	printf(" First Place Wins: %d Last place wins: %d\n", a->first_place_wins, a->last_place_wins);
	printf(" Caste: %d Default Room: %d\n", a->caste, a->default_room);
	printf(" Updates since last game played: %d\n", a->updates_since_last_game_played);

	return;
}

boolean empty_stats(
	struct player_stats *a)
{
	struct player_stats empty;
	boolean no_stats= FALSE;
	
	memset(&empty, 0, sizeof(struct player_stats));
	if(memcmp(a, &empty, sizeof(struct player_stats))==0)
	{
		no_stats= TRUE;
	}

	return no_stats;
}

