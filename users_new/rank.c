/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

 /*
  * The metaserver code changes that fall outside the original Bungie.net metaserver code 
  * license were written and are copyright 2002, 2003 of the following individuals:
  *
  * Copyright (c) 2002, 2003 Alan Wagner
  * Copyright (c) 2002 Vishvananda Ishaya
  * Copyright (c) 2003 Bill Keirstead
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
  */

#include "cseries.h"
#include "environment.h"
#include "stats.h"
#include "authentication.h"
#include "metaserver_common_structs.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "rank.h"

// ALAN Begin: added headers
#include <string.h> 
#include "orders.h"
// ALAN End

#define	NUMBER_OF_RANKING_PASSES				17

enum
{
	_overall_ranking = 0
};

enum
{
	_user_index_comet,
	_user_index_sun,
	_user_index_eclipsed_sun,
	_user_index_moon_1,
	_user_index_moon_0,
	_user_index_eclipsed_moon_2,
	_user_index_eclipsed_moon_1,
	_user_index_eclipsed_moon_0
};

float rank_percentages[] = 
{
	0.00f,
	0.00f,
	0.00f,
	0.16f,
	0.15f,
	0.14f,
	0.12f,
	0.11f,
	0.10f,
	0.09f,
	0.07f,
	0.06f
};

struct raw_rank_data
{
	unsigned long id;
	struct bungie_net_player_score_datum score;
};

static unsigned long allocate_ranking_data(
	struct raw_rank_data ** ranking_data, 
	boolean order,
	boolean * rank_data_allocated);

static void free_ranking_data(
	struct raw_rank_data * ranking_data, 
	boolean * rank_data_allocated);

static void calculate_rankings_for_gametype(
	unsigned long number_of_ranked_datums, 
	struct raw_rank_data * ranking_data, 
	short present_ranking, 
	boolean order,
	boolean * present_ranking_completed, 
	struct overall_ranking_data * overall_rank_data);

static boolean update_database_on_ranking(
	unsigned long number_of_ranked_datums, 
	struct raw_rank_data * ranking_data, 
	boolean order, 
	short * present_ranking);

static void calculate_caste_breakpoints(
	unsigned long number_of_ranked_datums, 
	struct raw_rank_data * ranking_data, 
	struct caste_breakpoint_data * caste_breakpoints);

static int compare_rankings(
	void * p0,
	void * p1);

boolean build_overall_ranking_data(
	struct caste_breakpoint_data * caste_breakpoints,
	struct overall_ranking_data * overall_rank_data)
{
	static boolean player_ranking_completed	= FALSE;
	static boolean rank_data_allocated = FALSE;
	static boolean present_ranking_completed = FALSE;
	static short present_ranking = 0;			// varies from 0 to 16 with 0 being overall

	static struct raw_rank_data * ranking_data;
	static unsigned long number_of_ranked_datums;

	boolean ret = FALSE;

	if (!player_ranking_completed)
	{
		switch (present_ranking)
		{
		case _overall_ranking:
			if (!rank_data_allocated)
			{
				number_of_ranked_datums = allocate_ranking_data(&ranking_data, player_ranking_completed, &rank_data_allocated);
				memset(overall_rank_data, 0, sizeof(struct overall_ranking_data));
			}

			if (rank_data_allocated)
			{
				calculate_rankings_for_gametype(number_of_ranked_datums, ranking_data, present_ranking, player_ranking_completed, &present_ranking_completed, overall_rank_data);
				if (present_ranking_completed)
				{
					calculate_caste_breakpoints(number_of_ranked_datums, ranking_data, caste_breakpoints);
					present_ranking_completed = !update_database_on_ranking(number_of_ranked_datums, ranking_data, player_ranking_completed, &present_ranking);
				}
			}
			break;

		default:
			if(present_ranking <= 0 || present_ranking >= NUMBER_OF_RANKING_PASSES)
				return FALSE;
			if(!rank_data_allocated)
				return FALSE;

			calculate_rankings_for_gametype(number_of_ranked_datums, ranking_data, present_ranking, player_ranking_completed, &present_ranking_completed, overall_rank_data);
			if (present_ranking_completed)
			{
				present_ranking_completed = !update_database_on_ranking(number_of_ranked_datums, ranking_data, player_ranking_completed, &present_ranking);

				if (present_ranking == NUMBER_OF_RANKING_PASSES)
				{
					free_ranking_data(ranking_data, &rank_data_allocated);
					number_of_ranked_datums = 0;
					player_ranking_completed = TRUE;
					present_ranking = _overall_ranking;
				}
			}
			break;
		}
	}
	else
	{	
		switch (present_ranking)
		{		
		case _overall_ranking:
			if (!rank_data_allocated)
			{
				number_of_ranked_datums = allocate_ranking_data(&ranking_data, player_ranking_completed, &rank_data_allocated);
			}

			if (rank_data_allocated)
			{
				calculate_rankings_for_gametype(number_of_ranked_datums, ranking_data, present_ranking, player_ranking_completed, &present_ranking_completed, overall_rank_data);
				if (present_ranking_completed)
				{
					present_ranking_completed = update_database_on_ranking(number_of_ranked_datums, ranking_data, player_ranking_completed, &present_ranking);
				}
			}
			break;

		default:
			if(present_ranking <= 0 || present_ranking >= NUMBER_OF_RANKING_PASSES)
				return FALSE;
			if(!rank_data_allocated)
				return FALSE;
			calculate_rankings_for_gametype(number_of_ranked_datums, ranking_data, present_ranking, player_ranking_completed, &present_ranking_completed, overall_rank_data);
			if (present_ranking_completed)
			{
				present_ranking_completed = update_database_on_ranking(number_of_ranked_datums, ranking_data, player_ranking_completed, &present_ranking);

				if (present_ranking == NUMBER_OF_RANKING_PASSES)
				{
					free_ranking_data(ranking_data, &rank_data_allocated);
					number_of_ranked_datums = 0;
					player_ranking_completed = FALSE;
					present_ranking = _overall_ranking;
					ret = TRUE;
				}
			}
			break;
		}
	}

	return ret;
}

static unsigned long allocate_ranking_data(
	struct raw_rank_data ** ranking_data, 
	boolean order,
	boolean * rank_data_allocated)
{
	unsigned long datum_count;

	if (!order)
	{
		datum_count = get_user_count();
	}
	else
	{
		datum_count = get_order_count();
	}

	// ALAN Begin: DBZ runtime error if !datum_count
	if( !datum_count ) 
		return 0;
	// ALAN End
	
	*ranking_data = malloc(sizeof(struct raw_rank_data) * datum_count);
	if (*ranking_data)
	{
		*rank_data_allocated = TRUE;
	}
	else
	{
		*rank_data_allocated = FALSE;
		datum_count = 0;
	}

	return datum_count;
}

static void free_ranking_data(
	struct raw_rank_data * ranking_data, 
	boolean * rank_data_allocated)
{
	free(ranking_data);

	*rank_data_allocated = FALSE;
}

#define	MAXIMUM_DATABASE_OPERATIONS_PER_CALL	1000
static void calculate_rankings_for_gametype(
	unsigned long number_of_ranked_datums, 
	struct raw_rank_data * ranking_data, 
	short present_ranking, 
	boolean order,
	boolean * present_ranking_completed, 
	struct overall_ranking_data * overall_rank_data)
{
	static unsigned long last_ranked_id = 1;

	assert(overall_rank_data);
	assert(ranking_data);
	assert(present_ranking_completed);
	assert(present_ranking < NUMBER_OF_RANKING_PASSES);

	*present_ranking_completed = FALSE;

	if (!order)
	{
		unsigned long n;
		struct bungie_net_player_datum player;

		for (n =0; (n < MAXIMUM_DATABASE_OPERATIONS_PER_CALL) && (last_ranked_id <= number_of_ranked_datums); 
			++n, ++last_ranked_id)
		{
			get_player_information(NULL, last_ranked_id, &player);
			ranking_data[last_ranked_id - 1].id = last_ranked_id;

			switch (present_ranking)
			{
			case _overall_ranking:
				ranking_data[last_ranked_id - 1].score = player.ranked_score;
				
				if (player.ranked_score.points > overall_rank_data->ranked_game_data.points.best)
				{
					overall_rank_data->ranked_game_data.points.best = player.ranked_score.points;
				}
				overall_rank_data->ranked_game_data.points.average += player.ranked_score.points;

				if (player.ranked_score.games_played > overall_rank_data->ranked_game_data.games_played.best)
				{
					overall_rank_data->ranked_game_data.games_played.best = player.ranked_score.games_played;
				}
				overall_rank_data->ranked_game_data.games_played.average += player.ranked_score.games_played;

				if (player.ranked_score.wins > overall_rank_data->ranked_game_data.wins.best)
				{
					overall_rank_data->ranked_game_data.wins.best = player.ranked_score.wins;
				}
				overall_rank_data->ranked_game_data.wins.average += player.ranked_score.wins;

				if (player.ranked_score.damage_inflicted > overall_rank_data->ranked_game_data.damage_inflicted.best)
				{
					overall_rank_data->ranked_game_data.damage_inflicted.best = player.ranked_score.damage_inflicted;
				}
				overall_rank_data->ranked_game_data.damage_inflicted.average += player.ranked_score.damage_inflicted;

				if (player.ranked_score.damage_received > overall_rank_data->ranked_game_data.damage_received.best)
				{
					overall_rank_data->ranked_game_data.damage_received.best = player.ranked_score.damage_received;
				}
				overall_rank_data->ranked_game_data.damage_received.average += player.ranked_score.damage_received;
				break;

			default:
				ranking_data[last_ranked_id - 1].score = player.ranked_scores_by_game_type[present_ranking - 1];

				if (player.ranked_scores_by_game_type[present_ranking - 1].points > 
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].points.best)
				{
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].points.best = 
						player.ranked_scores_by_game_type[present_ranking - 1].points;
				}
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].points.average += 
					player.ranked_scores_by_game_type[present_ranking - 1].points;

				if (player.ranked_scores_by_game_type[present_ranking - 1].games_played > 
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].games_played.best)
				{
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].games_played.best = 
						player.ranked_scores_by_game_type[present_ranking - 1].games_played;
				}
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].games_played.average += 
					player.ranked_scores_by_game_type[present_ranking - 1].games_played;

				if (player.ranked_scores_by_game_type[present_ranking - 1].wins > 
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].wins.best)
				{
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].wins.best = 
						player.ranked_scores_by_game_type[present_ranking - 1].wins;
				}
				overall_rank_data->ranked_game_data_by_game_type[present_ranking -1].wins.average += 
					player.ranked_scores_by_game_type[present_ranking - 1].wins;

				if (player.ranked_scores_by_game_type[present_ranking - 1].damage_inflicted > 
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.best)
				{
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.best = 
						player.ranked_scores_by_game_type[present_ranking - 1].damage_inflicted;
				}
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.average += 
					player.ranked_scores_by_game_type[present_ranking - 1].damage_inflicted;

				if (player.ranked_scores_by_game_type[present_ranking - 1].damage_received > 
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_received.best)
				{
					overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_received.best = 
						player.ranked_scores_by_game_type[present_ranking - 1].damage_received;
				}
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_received.average += 
					player.ranked_scores_by_game_type[present_ranking - 1].damage_received;				

				break;
			}
		}
	}
	else
	{
		unsigned long n;
		struct bungie_net_order_datum order;

		for (n =0; (n < MAXIMUM_DATABASE_OPERATIONS_PER_CALL) && (last_ranked_id <= number_of_ranked_datums); 
			++n, ++last_ranked_id)
		{
			get_order_information(NULL, last_ranked_id, &order);
			ranking_data[last_ranked_id - 1].id = order.order_id;

			switch (present_ranking)
			{
			case _overall_ranking:
				ranking_data[last_ranked_id - 1].score = order.ranked_score;
				
				if (order.ranked_score.points > overall_rank_data->order_ranked_game_data.points.best)
				{
					overall_rank_data->order_ranked_game_data.points.best = order.ranked_score.points;
				}
				overall_rank_data->order_ranked_game_data.points.average += order.ranked_score.points;

				if (order.ranked_score.games_played > overall_rank_data->order_ranked_game_data.games_played.best)
				{
					overall_rank_data->order_ranked_game_data.games_played.best = order.ranked_score.games_played;
				}
				overall_rank_data->order_ranked_game_data.games_played.average += order.ranked_score.games_played;

				if (order.ranked_score.wins > overall_rank_data->order_ranked_game_data.wins.best)
				{
					overall_rank_data->order_ranked_game_data.wins.best = order.ranked_score.wins;
				}
				overall_rank_data->order_ranked_game_data.wins.average += order.ranked_score.wins;

				if (order.ranked_score.damage_inflicted > overall_rank_data->order_ranked_game_data.damage_inflicted.best)
				{
					overall_rank_data->order_ranked_game_data.damage_inflicted.best = order.ranked_score.damage_inflicted;
				}
				overall_rank_data->order_ranked_game_data.damage_inflicted.average += order.ranked_score.damage_inflicted;

				if (order.ranked_score.damage_received > overall_rank_data->order_ranked_game_data.damage_received.best)
				{
					overall_rank_data->order_ranked_game_data.damage_received.best = order.ranked_score.damage_received;
				}
				overall_rank_data->order_ranked_game_data.damage_received.average += order.ranked_score.damage_received;
				break;

			default:
				ranking_data[last_ranked_id - 1].score = order.ranked_scores_by_game_type[present_ranking - 1];

				if (order.ranked_scores_by_game_type[present_ranking - 1].points > 
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].points.best)
				{
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].points.best = 
						order.ranked_scores_by_game_type[present_ranking - 1].points;
				}
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].points.average += 
					order.ranked_scores_by_game_type[present_ranking - 1].points;

				if (order.ranked_scores_by_game_type[present_ranking - 1].games_played > 
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].games_played.best)
				{
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].games_played.best = 
						order.ranked_scores_by_game_type[present_ranking - 1].games_played;
				}
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].games_played.average += 
					order.ranked_scores_by_game_type[present_ranking - 1].games_played;

				if (order.ranked_scores_by_game_type[present_ranking - 1].wins > 
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].wins.best)
				{
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].wins.best = 
						order.ranked_scores_by_game_type[present_ranking - 1].wins;
				}
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking -1].wins.average += 
					order.ranked_scores_by_game_type[present_ranking - 1].wins;

				if (order.ranked_scores_by_game_type[present_ranking - 1].damage_inflicted > 
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.best)
				{
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.best = 
						order.ranked_scores_by_game_type[present_ranking - 1].damage_inflicted;
				}
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.average += 
					order.ranked_scores_by_game_type[present_ranking - 1].damage_inflicted;

				if (order.ranked_scores_by_game_type[present_ranking - 1].damage_received > 
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_received.best)
				{
					overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_received.best = 
						order.ranked_scores_by_game_type[present_ranking - 1].damage_received;
				}
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_received.average += 
					order.ranked_scores_by_game_type[present_ranking - 1].damage_received;
				break;
			}
		}
	}

	if (last_ranked_id > number_of_ranked_datums)
	{ 
		unsigned long x;

		// ALAN Begin: passing arg 4 of 'qsort' from incompatible pointer type fix
	//	qsort(ranking_data, number_of_ranked_datums, sizeof(struct raw_rank_data), compare_rankings);
		qsort(ranking_data, number_of_ranked_datums, sizeof(struct raw_rank_data), (int(*)(const void*, const void*))compare_rankings);
		// ALAN End

		for (x = 0; x < number_of_ranked_datums; ++x)
		{
			ranking_data[x].score.numerical_rank = x + 1;
		}

		if (!order)
		{
			struct bungie_net_player_datum player;

			get_player_information(NULL, ranking_data[0].id, &player);
			switch (present_ranking)
			{
			case _overall_ranking:
				strcpy(overall_rank_data->ranked_game_data.top_ranked_player, player.name);

				overall_rank_data->total_users = number_of_ranked_datums;

				overall_rank_data->ranked_game_data.points.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data.games_played.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data.wins.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data.damage_inflicted.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data.damage_received.average /= number_of_ranked_datums;
				break;

			default:
				strcpy(overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].top_ranked_player, player.name);

				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].points.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].games_played.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].wins.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.average /= number_of_ranked_datums;
				overall_rank_data->ranked_game_data_by_game_type[present_ranking - 1].damage_received.average /= number_of_ranked_datums;
				break;
			}
		}
		else
		{
			struct bungie_net_order_datum order;

			get_order_information(NULL, ranking_data[0].id, &order);
			switch (present_ranking)
			{
			case _overall_ranking:
				strcpy(overall_rank_data->order_ranked_game_data.top_ranked_player, order.name);

				overall_rank_data->total_orders = number_of_ranked_datums;

				overall_rank_data->order_ranked_game_data.points.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data.games_played.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data.wins.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data.damage_inflicted.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data.damage_received.average /= number_of_ranked_datums;
				break;

			default:
				strcpy(overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].top_ranked_player, order.name);

				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].points.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].games_played.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].wins.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_inflicted.average /= number_of_ranked_datums;
				overall_rank_data->order_ranked_game_data_by_game_type[present_ranking - 1].damage_received.average /= number_of_ranked_datums;
				break;
			}
		}

		last_ranked_id = 1;
		*present_ranking_completed = TRUE;
	}
}

static boolean update_database_on_ranking(
	unsigned long number_of_ranked_datums, 
	struct raw_rank_data * ranking_data, 
	boolean order, 
	short * present_ranking)
{
	static unsigned long last_ranked_id = 1;
	boolean ret = FALSE;

	assert(ranking_data);
	assert(present_ranking);
	if(*present_ranking < 0 && *present_ranking >= NUMBER_OF_RANKING_PASSES)
		return FALSE;

	if (!order)
	{
		unsigned long n;
		struct bungie_net_player_datum player;

		if(number_of_ranked_datums > get_user_count())
			return FALSE;
		for (n =0; (n < MAXIMUM_DATABASE_OPERATIONS_PER_CALL) && (last_ranked_id <= number_of_ranked_datums); 
			++n, ++last_ranked_id)
		{
			get_player_information(NULL, ranking_data[last_ranked_id - 1].id, &player);
			switch (*present_ranking)
			{
			case _overall_ranking:
				player.ranked_score = ranking_data[last_ranked_id - 1].score;
				break;

			default:
				player.ranked_scores_by_game_type[*present_ranking - 1] = ranking_data[last_ranked_id - 1].score;
				break;
			}

			update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
		}
	}
	else
	{
		unsigned long n;
		struct bungie_net_order_datum order;

		if(number_of_ranked_datums > get_order_count())
			return FALSE;

		for (n =0; (n < MAXIMUM_DATABASE_OPERATIONS_PER_CALL) && (last_ranked_id <= number_of_ranked_datums); 
			++n, ++last_ranked_id)
		{
			get_order_information(NULL, ranking_data[last_ranked_id - 1].id , &order);
			switch (*present_ranking)
			{
			case _overall_ranking:
				order.ranked_score = ranking_data[last_ranked_id - 1].score;
				break;

			default:
				order.ranked_scores_by_game_type[*present_ranking - 1] = ranking_data[last_ranked_id - 1].score;
				break;
			}

			update_order_information(NULL, order.order_id, &order);
		}
	}

	if (last_ranked_id > number_of_ranked_datums)
	{
		last_ranked_id = 1;
		(*present_ranking)++;
		ret = TRUE;
	}

	return ret;
}

static void calculate_caste_breakpoints(
	unsigned long number_of_ranked_datums, 
	struct raw_rank_data * ranking_data, 
	struct caste_breakpoint_data * caste_breakpoints)
{
	unsigned long n;
	unsigned long active_player_count = 0;

	for (n = 0; n < number_of_ranked_datums; ++n)
	{
		if (ranking_data[n].score.games_played > GAMES_PLAYED_KRIS_DAGGER_CASTE)
		{
			active_player_count++;
		}
		else
		{
			if (ranking_data[n].score.games_played < GAMES_PLAYED_DAGGER_CASTE)
			{
				ranking_data[n].score.rank = _dagger;
			}
			else if ((ranking_data[n].score.games_played >= GAMES_PLAYED_DAGGER_CASTE) &&
				(ranking_data[n].score.games_played < GAMES_PLAYED_DAGGER_WITH_HILT_CASTE))
			{
				ranking_data[n].score.rank = _dagger_with_hilt;
			}
			else
			{
				ranking_data[n].score.rank = _kris_knife;
			}
		}
	}

	if (active_player_count)
	{
		unsigned long members_in_present_caste = 0;
		short present_caste = _eclipsed_moon;

		for (n = 0; n < active_player_count; ++n)
		{
			switch (n)
			{
			case _user_index_comet:
				caste_breakpoints->comet_player_ids[0] = ranking_data[n].id;
				ranking_data[n].score.rank = _comet;
				break;

			case _user_index_sun:
				caste_breakpoints->sun_player_ids[0] = ranking_data[n].id;
				ranking_data[n].score.rank = _sun;
				break;

			case _user_index_eclipsed_sun:
				caste_breakpoints->eclipsed_sun_player_ids[0] = ranking_data[n].id;
				ranking_data[n].score.rank = _eclipsed_sun;
				break;

			case _user_index_moon_0:
				caste_breakpoints->moon_player_ids[0] = ranking_data[n].id;
				ranking_data[n].score.rank = _moon;
				break;

			case _user_index_moon_1:
				caste_breakpoints->moon_player_ids[1] = ranking_data[n].id;
				ranking_data[n].score.rank = _moon;
				break;

			case _user_index_eclipsed_moon_0:
				caste_breakpoints->eclipsed_moon_player_ids[0] = ranking_data[n].id;
				ranking_data[n].score.rank = _eclipsed_moon;
				break;

			case _user_index_eclipsed_moon_1:
				caste_breakpoints->eclipsed_moon_player_ids[1] = ranking_data[n].id;
				ranking_data[n].score.rank = _eclipsed_moon;
				break;

			case _user_index_eclipsed_moon_2:
				caste_breakpoints->eclipsed_moon_player_ids[2] = ranking_data[n].id;
				ranking_data[n].score.rank = _eclipsed_moon;
				break;

			default:
				if (!members_in_present_caste)
				{
					present_caste--;
					members_in_present_caste = rank_percentages[present_caste] * active_player_count;
					caste_breakpoints->normal_caste_breakpoints[present_caste - _sword_and_dagger] =
						ranking_data[n - 1].score.points;
				}

				ranking_data[n].score.rank = present_caste;
				members_in_present_caste--;
				break;
			}
		}
	}
}

static int compare_rankings(
	void * p0,
	void * p1)
{
	struct raw_rank_data * r0, * r1;

	int ret;

	r0 = (struct raw_rank_data *)p0;
	r1 = (struct raw_rank_data *)p1;

	if ((r0->score.games_played > GAMES_PLAYED_KRIS_DAGGER_CASTE) &&
		(r1->score.games_played <= GAMES_PLAYED_KRIS_DAGGER_CASTE))
	{
		ret = -1;
	}
	else if ((r1->score.games_played > GAMES_PLAYED_KRIS_DAGGER_CASTE) &&
		(r1->score.games_played <= GAMES_PLAYED_KRIS_DAGGER_CASTE))
	{
		ret = 1;
	}
	else
	{
		if (r0->score.points > r1->score.points)
		{
			ret = -1;
		}
		else if (r1->score.points > r0->score.points)
		{
			ret = 1;
		}
		else if (r0->score.games_played > r1->score.games_played)
		{
			ret = -1;
		}
		else if (r1->score.games_played > r0->score.games_played)
		{
			ret = 1;
		}
		else
		{
			ret = 0;
		}
	}

	return ret;
}
