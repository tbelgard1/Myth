/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include "environment.h"
#include "metaserver_common_structs.h"
#include "rb_tree.h"
#include "sl_list.h"
#include "stats.h"
#include "byte_swapping.h"
#include "bungie_net_player.h"
#include "users.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define	BUNGIE_NET_USER_DB_SIGNATURE		'PLAY'

struct order_member_list_data
{
	unsigned long player_id;
};

struct order_list_data
{
	short order_index;
	struct sl_list *member_list;
};

struct bungie_net_user_db_header
{
	unsigned long player_count;
	unsigned long unused[40];
};

struct bungie_net_user_db_entry
{
	unsigned long signature;
	struct bungie_net_player_datum player;
};

struct bungie_net_login_tree_data
{
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	unsigned long online_data_index;
	long fpos;
};

static boolean lock_file_region(
	long offset, 
	long length,
	boolean write,
	boolean block);

static boolean unlock_file_region(
	long offset,
	long length);

static boolean place_player_in_order(
	short order_index,
	unsigned long player_id);

static void remove_player_from_order(
	short order_index,
	unsigned long player_id);

static boolean order_list_new(
	void);

static struct sl_list_element *order_new(
	struct sl_list *order_list, 
	short order_index);

static void order_dispose(
	struct sl_list *order_list,
	struct sl_list_element *order_element);

static struct sl_list_element *order_member_new(
	struct sl_list *member_list, 
	unsigned long player_id);

static void order_member_dispose(
	struct sl_list *member_list,
	struct sl_list_element *member_element);

void order_member_list_dispose(
	struct sl_list *member_list);

static int order_list_comp_func(
	void *k0,
	void *k1);

static int order_member_list_comp_func(
	void *k0,
	void *k1);

static boolean add_entry_to_login_tree(
	struct bungie_net_player_datum * player,
	unsigned long fpos);

static int login_tree_comp_func(
	void * k0,
	void * k1);

static int compare_buddies(
	const void * p1, 
	const void * p2);

static void make_packed_player_data(
	char * player_data,
	short * length,
	struct bungie_net_player_datum * player);

static void refresh_response_list(
	void);

static void add_player_to_response_list(
	struct bungie_net_online_player_data * opd);

static char * ci_pn_match(
	char * s0,
	char * s1);

static struct rb_tree bungie_net_login_tree;
static struct sl_list * order_list;

static struct bungie_net_online_player_data * online_player_data;
static int fd_user_db = -1;
static unsigned long total_players;
static struct user_query_response response_list[MAXIMUM_PLAYER_SEARCH_RESPONSES];


boolean create_user_database(
	void)
{
	struct bungie_net_user_db_header header;
	char * file_name;
	boolean success = FALSE;

	memset(&header, 0, sizeof(struct bungie_net_user_db_header));
	file_name = get_users_db_file_name();
	fd_user_db = open(file_name, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fd_user_db != -1)
	{
		if (write(fd_user_db, &header, sizeof(struct bungie_net_user_db_header)) != -1)
		{
			success = TRUE;
		}
		else
		{
			close(fd_user_db);
			fd_user_db = -1;
		}
	}

	return success;
}

boolean initialize_user_database(
	void)
{
	long fpos;
	unsigned long player_count = 0;
	char * file_name;
	boolean success = FALSE;
	struct bungie_net_user_db_header header;
	struct bungie_net_user_db_entry user;
	
	strcpy(bungie_net_login_tree.name, "login tree");
	bungie_net_login_tree.root_element = NULL;
	bungie_net_login_tree.comp_func = login_tree_comp_func;

	if (order_list_new())
	{
		file_name = get_users_db_file_name();
		fd_user_db = open(file_name, O_RDWR, 0);
		if (fd_user_db != -1)
		{
			int length;

			length = read(fd_user_db, &header, sizeof(struct bungie_net_user_db_header));
			fpos = length;
			if (length == sizeof(struct bungie_net_user_db_header))
			{
				int player_index;

				player_count = header.player_count;
				online_player_data = malloc(sizeof(struct bungie_net_online_player_data) * player_count);
				if (online_player_data)
				{
					total_players = player_count;
					for (player_index = 0; player_index < player_count; player_index++)
					{
						if (lseek(fd_user_db, sizeof(struct bungie_net_user_db_header) + 
							(player_index * sizeof(struct bungie_net_user_db_entry)), SEEK_SET) != -1)
						{
							length = read(fd_user_db, &user, sizeof(struct bungie_net_user_db_entry));
							if (user.signature != BUNGIE_NET_USER_DB_SIGNATURE)
							{
								char buffer[128];
								sprintf(buffer, "invalid signature in user database, offset = %d", fpos);
								vhalt(buffer);
							}

							online_player_data[player_index].online_data_index = player_index;
							online_player_data[player_index].player_id = user.player.player_id;
							strcpy(online_player_data[player_index].login, user.player.login);
							strcpy(online_player_data[player_index].name, user.player.name);
							online_player_data[player_index].room_id = user.player.room_id;
							online_player_data[player_index].order = user.player.order_index;
							make_packed_player_data(online_player_data[player_index].player_data,
								&online_player_data[player_index].aux_data.player_data_length, &user.player);
							online_player_data[player_index].logged_in_flag = FALSE;
							online_player_data[player_index].aux_data.verb = 0;
							online_player_data[player_index].aux_data.flags = 0;
							online_player_data[player_index].aux_data.ranking = user.player.ranked_score.numerical_rank;
							online_player_data[player_index].aux_data.caste = user.player.ranked_score.rank;
							online_player_data[player_index].aux_data.player_id = user.player.player_id;
							online_player_data[player_index].aux_data.room_id = user.player.room_id;

							online_player_data[player_index].fpos = fpos;
		
							if (!place_player_in_order(user.player.order_index, user.player.player_id))
							{
								break;
							}

							if (!add_entry_to_login_tree(&user.player, fpos))
							{
								break;
							}

							fpos += length;
						}
					}
					if (player_index == player_count)
					{
						success = TRUE;
					}
				}
			}
		}
	}
	
	return success;
}

void shutdown_user_database(
	void)
{
	return;
}

struct sl_list_element *present_order_list;
unsigned long get_first_player_in_order(
	short order_index,
	void **key)
{
	struct sl_list_element *order_element;
	unsigned long ret= 0;

	if(!key)
		return ret;

	*key= NULL;
	present_order_list= NULL;

	if (order_index)
	{
		order_element= sl_list_search_for_element(order_list, &order_index);	
		if (order_element)
		{
			struct sl_list_element *member_element;
			struct order_list_data *order_data;
			struct order_member_list_data *member_data;

			present_order_list= order_element;

			order_data= (struct order_list_data *)order_element->data;

			member_element= sl_list_get_head_element(order_data->member_list);
			if (member_element)
			{
				*key= (void *)member_element;

				member_data= (struct order_member_list_data *)member_element->data;
				ret= member_data->player_id;
			}
		}
	}

	return ret;
}

unsigned long get_next_player_in_order(
	void **key)
{
	struct sl_list_element *member_element;
	unsigned long ret= 0;

	struct order_list_data *order_data= (struct order_list_data *)present_order_list->data;
	
	member_element= sl_list_get_next_element(order_data->member_list, (struct sl_list_element *)(*key));
	if (member_element)
	{
		struct order_member_list_data *member_data;

		member_data= (struct order_member_list_data *)member_element->data;
		ret= member_data->player_id;
		
		*key= (void *)member_element;
	}
	else
	{
		*key= NULL;
	}

	return ret;
}

unsigned long get_user_count(void)
{
	struct bungie_net_user_db_header header;
	unsigned long user_count = 0;

	if (lseek(fd_user_db, 0, SEEK_SET) != -1)
	{
		if (read(fd_user_db, &header, sizeof(struct bungie_net_user_db_header)) != -1)
		{
			user_count = header.player_count;
		}
	}

	return user_count;
}

boolean get_online_player_information(
	unsigned long player_id,
	struct bungie_net_online_player_data * player)
{
	boolean success = FALSE;

	if (player_id < total_players)
	{
		*player = online_player_data[player_id - 1];
		success = TRUE;
	}

	return success;
}

static unsigned long search_player_id = -1;
boolean get_first_player_information(
	struct bungie_net_player_datum * player)
{
	search_player_id = 1;

	return get_player_information(NULL, search_player_id, player);
}

boolean get_next_player_information(
	struct bungie_net_player_datum * player)
{
	++search_player_id;

	return get_player_information(NULL, search_player_id, player);
}

boolean get_player_information(
	char * login_name,
	unsigned long player_id,
	struct bungie_net_player_datum * player)
{
	int length;
	struct bungie_net_user_db_entry player_entry;
	struct element * element;
	boolean success = FALSE;
	long filepos = -1;

	if(!((login_name && !player_id) || (!login_name && player_id)))
		return FALSE;
	if(!player)
		return FALSE;

	if ((player_id > 0) && (player_id <= total_players))
	{
		filepos = online_player_data[player_id - 1].fpos;
	}
	else if (login_name)
	{
		struct bungie_net_login_tree_data * data;

		element = search_for_element(&bungie_net_login_tree, login_name);
		if (element)
		{
			data = element->data;
			filepos = data->fpos;
		}
	}
	
	if (filepos != -1)
	{
		if (lseek(fd_user_db, filepos, SEEK_SET) != -1)
		{
			length = read(fd_user_db, &player_entry, sizeof(struct bungie_net_user_db_entry));
			if (length == sizeof(struct bungie_net_user_db_entry) &&
				player_entry.signature == BUNGIE_NET_USER_DB_SIGNATURE)
			{
				memcpy(player, &player_entry.player, sizeof(struct bungie_net_player_datum));
				success = TRUE;
			}
		}
	}

	return success;
}

boolean update_player_information(
	char * login_name,
	unsigned long player_id,
	boolean logged_in_flag,
	struct bungie_net_player_datum * player)
{
	struct bungie_net_user_db_entry entry;
	struct element * element;
	int length;
	long filepos = -1;
	boolean success = FALSE;

	if(!((login_name && !player_id) || (!login_name && player_id)))
		return FALSE;
	if(!player)
		return FALSE;
	if (player_id)
	{
		filepos = online_player_data[player_id - 1].fpos;
	}
	else
	{
		struct bungie_net_login_tree_data * data;

		element = search_for_element(&bungie_net_login_tree, login_name);
		if (element)
		{
			data = element->data;
			filepos = data->fpos;
		}
	}

	if (lseek(fd_user_db, filepos, SEEK_SET) != -1)
	{
		entry.signature = BUNGIE_NET_USER_DB_SIGNATURE;
		
		length = read(fd_user_db, &entry, sizeof(struct bungie_net_user_db_entry));
		if(lseek(fd_user_db, filepos, SEEK_SET) != -1)
		{
			if (player->order_index != entry.player.order_index) 
				remove_player_from_order(entry.player.order_index, entry.player.player_id);
			
			entry.player = *player;
			
			length = write(fd_user_db, &entry, sizeof(struct bungie_net_user_db_entry));
			if (length == sizeof(struct bungie_net_user_db_entry))
			{
				unsigned long player_index = player->player_id - 1;
				
				online_player_data[player_index].online_data_index = player_index;
				online_player_data[player_index].player_id = player->player_id;
				strcpy(online_player_data[player_index].login, player->login);
				strcpy(online_player_data[player_index].name, player->name);
				online_player_data[player_index].room_id = player->room_id;
				online_player_data[player_index].order = player->order_index;
				make_packed_player_data(online_player_data[player_index].player_data, 
					&online_player_data[player_index].aux_data.player_data_length, player);
				online_player_data[player_index].logged_in_flag = logged_in_flag;
				
				online_player_data[player_index].aux_data.verb = 0;
				online_player_data[player_index].aux_data.flags = 0;
				online_player_data[player_index].aux_data.ranking = player->ranked_score.numerical_rank;
				online_player_data[player_index].aux_data.caste = player->ranked_score.rank;
				online_player_data[player_index].aux_data.player_id = player->player_id;
				online_player_data[player_index].aux_data.room_id = player->room_id;
				
				place_player_in_order(player->order_index, player->player_id);
				success = TRUE;
			}
		}
	}

	return success;
}

static int compare_buddies(
	const void * p1, 
	const void * p2)
{
	struct buddy_entry * b1, * b2;

	b1 = (struct buddy_entry *)p1;
	b2 = (struct buddy_entry *)p2;

	if (b1->active > b2->active)
		return -1;
	else if (b1->active == b2->active)
		return 0;

	return 1;
}

static boolean player_in_buddy_list(
	unsigned long player_id,
	struct buddy_entry * buddy_list)
{
	short n;

	for (n = 0; n < MAXIMUM_BUDDIES; ++n)
	{
		if ((buddy_list[n].active != INACTIVE) && (buddy_list[n].player_id == player_id))
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void modify_buddy_list(
	unsigned long player_id,
	struct buddy_entry * buddy_list,
	char state)
{
	short n;

	for (n = 0; n < MAXIMUM_BUDDIES; ++n)
	{
		if (buddy_list[n].active == INACTIVE)
		{
			if (state != INACTIVE)
			{
				buddy_list[n].player_id = player_id;
				buddy_list[n].active = state;
				break;
			}
			else
			{
				break;
			}
		}

		if (buddy_list[n].player_id == player_id)
		{
			if (state == INACTIVE)
			{
				buddy_list[n].player_id = 0;
			}

			buddy_list[n].active = state;
			break;
		}
	}
}

void update_buddy_list(
	unsigned long player_id,
	unsigned long buddy_id,
	boolean add)
{
	struct bungie_net_player_datum player_user_data, buddy_user_data;
	boolean update_player = FALSE, update_buddy = FALSE;
	
	if (get_player_information(NULL, player_id, &player_user_data))
	{
		if (get_player_information(NULL, buddy_id, &buddy_user_data))
		{
			if (add)
			{
				if (player_in_buddy_list(buddy_id, player_user_data.buddies))
				{
					if (player_in_buddy_list(player_id, buddy_user_data.buddies))
					{
						modify_buddy_list(buddy_id, player_user_data.buddies, ACTIVE);
						modify_buddy_list(player_id, buddy_user_data.buddies, ACTIVE);
						update_player = TRUE;
						update_buddy = TRUE;
					}
				}
				else
				{
					if (player_in_buddy_list(player_id, buddy_user_data.buddies))
					{
						modify_buddy_list(buddy_id, player_user_data.buddies, ACTIVE);
						modify_buddy_list(player_id, buddy_user_data.buddies, ACTIVE);
						update_player = TRUE;
						update_buddy = TRUE;
					}
					else
					{
						modify_buddy_list(buddy_id, player_user_data.buddies, UNACKNOWLEDGED);
						update_player = TRUE;
					}
				}
			}
			else
			{
				if (player_in_buddy_list(buddy_id, player_user_data.buddies))
				{
					modify_buddy_list(buddy_id, player_user_data.buddies, INACTIVE);
					update_player = TRUE;

					if (player_in_buddy_list(player_id, buddy_user_data.buddies))
					{
						modify_buddy_list(player_id, buddy_user_data.buddies, UNACKNOWLEDGED);
						update_buddy = TRUE;
					}
				}
			}

			if (update_player)
			{
				qsort(&player_user_data.buddies[0], MAXIMUM_BUDDIES, sizeof(struct buddy_entry), compare_buddies);
				update_player_information(NULL, player_id, is_player_online(player_id), &player_user_data);
			}

			if (update_buddy)
			{
				qsort(&buddy_user_data.buddies[0], MAXIMUM_BUDDIES, sizeof(struct buddy_entry), compare_buddies);
				update_player_information(NULL, buddy_id, is_player_online(buddy_id), &buddy_user_data);
			}
		}
	}
}

void update_order(
	unsigned long player_id, 
	short order)
{
	struct bungie_net_player_datum user_data;
	
	get_player_information(NULL, player_id, &user_data);
	if (user_data.order_index == order)
		return;

	user_data.order_index = order;
	update_player_information(NULL, player_id, online_player_data[player_id - 1].logged_in_flag, &user_data);
}

boolean new_user(
	struct bungie_net_player_datum * player)
{
	struct bungie_net_user_db_header header;
	struct bungie_net_user_db_entry entry;
	boolean success = FALSE;
	long fpos, user_fpos;

	struct element * element;

	element = search_for_element(&bungie_net_login_tree, (void *)player->login);
	if (element)
	{
		return FALSE;
	}

	entry.signature = BUNGIE_NET_USER_DB_SIGNATURE;	
	fpos = lseek(fd_user_db, 0, SEEK_SET);
	if (fpos != -1)
	{
		if (read(fd_user_db, &header, sizeof(struct bungie_net_user_db_header)) != -1)
		{
			header.player_count++;
			player->player_id = header.player_count;
			player->order_index= 0;
			memset(&player->buddies[0], 0, sizeof(struct buddy_entry)*MAXIMUM_BUDDIES);
			entry.player = *player;

			if (lseek(fd_user_db, sizeof(struct bungie_net_user_db_header) + 
				((header.player_count - 1) * sizeof(struct bungie_net_user_db_entry)), SEEK_SET) != -1)
			{
				if (write(fd_user_db, &entry, sizeof(struct bungie_net_user_db_entry)) != -1)
				{
					user_fpos = sizeof(struct bungie_net_user_db_header) +
						((header.player_count - 1) * sizeof(struct bungie_net_user_db_entry));
					fpos = lseek(fd_user_db, 0, SEEK_SET);
					if (fpos != -1)
					{
						if (write(fd_user_db, &header, sizeof(struct bungie_net_user_db_header)) != -1)
						{
							struct bungie_net_online_player_data * opd;

							opd = realloc(online_player_data, 
								header.player_count * sizeof(struct bungie_net_online_player_data));
							if (opd)
							{
								online_player_data = opd;
								online_player_data[header.player_count - 1].online_data_index = header.player_count - 1;
								online_player_data[header.player_count - 1].player_id = player->player_id;
								strcpy(online_player_data[header.player_count - 1].login, player->login);
								strcpy(online_player_data[header.player_count - 1].name, player->name);
								online_player_data[header.player_count - 1].room_id = player->room_id;
								online_player_data[header.player_count - 1].order = player->order_index;
								make_packed_player_data(online_player_data[header.player_count - 1].player_data, 
									&online_player_data[header.player_count - 1].aux_data.player_data_length, player);
								online_player_data[header.player_count - 1].logged_in_flag = FALSE;
								online_player_data[header.player_count - 1].fpos = user_fpos;
								online_player_data[header.player_count - 1].aux_data.verb = 0;
								online_player_data[header.player_count - 1].aux_data.flags = 0;
								online_player_data[header.player_count - 1].aux_data.ranking = player->ranked_score.numerical_rank;
								online_player_data[header.player_count - 1].aux_data.caste = player->ranked_score.rank;
								online_player_data[header.player_count - 1].aux_data.player_id = player->player_id;
								online_player_data[header.player_count - 1].aux_data.room_id = player->room_id;

								total_players = header.player_count;

								if (add_entry_to_login_tree(player, user_fpos))
								{
									if (place_player_in_order(player->order_index, player->player_id))
									{
										success = TRUE;
									}
									else
									{
									}
								}
								else
								{
								}
							}
							else
							{
							}
						}
					}
				}
			}
		}
	}

	return success;
}

void query_user_database(
	struct user_query * query,
	struct user_query_response ** query_response)
{
	unsigned long index;

	refresh_response_list();

	if (query->string[0])
	{
		for (index = 0; index < total_players; index++)
		{
			if (ci_pn_match(online_player_data[index].name, query->string) &&
				online_player_data[index].logged_in_flag)
			{
				add_player_to_response_list(&online_player_data[index]);
			}
			else if (ci_pn_match(online_player_data[index].login, query->string) &&
				online_player_data[index].logged_in_flag)
			{
				add_player_to_response_list(&online_player_data[index]);
			}
		}
	}
	else if (query->order)
	{
		unsigned long player_id;
		void *key;

		player_id = get_first_player_in_order(query->order, &key);
		while (key)
		{
			add_player_to_response_list(&online_player_data[player_id - 1]);
			player_id = get_next_player_in_order(&key);
		}
	}
	else if (query->buddy_ids[0])
	{
		short num_buddies;
		short buddy_index;

		for (num_buddies = 0; num_buddies < MAXIMUM_BUDDIES; num_buddies++) if (!query->buddy_ids[num_buddies]) break;

		for (buddy_index = 0; buddy_index < num_buddies; buddy_index++)
		{
			if (query->buddy_ids[buddy_index] <= total_players) 
				add_player_to_response_list(&online_player_data[query->buddy_ids[buddy_index] - 1]);
		}
	}

	*query_response = &response_list[0];
}

boolean is_player_online(
	unsigned long player_id)
{
	boolean ret;

	if (player_id > 0 && player_id <= get_user_count())
		ret = online_player_data[player_id - 1].logged_in_flag;
	else
		ret = FALSE;

	return ret;
}

short get_player_count_in_order(
	short order_index)
{
	struct sl_list_element *order_element;
	short ret= 0;

	order_element= sl_list_search_for_element(order_list, &order_index);
	if (order_element)
	{
		struct order_list_data *order_data;
		struct sl_list_element *member_element;

		order_data= (struct order_list_data *)order_element->data;
		for (member_element= sl_list_get_head_element(order_data->member_list); member_element; member_element= sl_list_get_next_element(order_data->member_list, member_element)) ++ret;
	}

	return ret;
}

static boolean place_player_in_order(
	short order_index,
	unsigned long player_id)
{
	boolean ret= FALSE;

	if (order_index)
	{
		struct sl_list_element *order_element;

		order_element= sl_list_search_for_element(order_list, &order_index);
		if (order_element)
		{
			struct order_list_data *order_data;

			order_data= (struct order_list_data *)order_element->data;
			if (!sl_list_search_for_element(order_data->member_list, &player_id))
			{
				struct sl_list_element *member_element;

				member_element= order_member_new(order_data->member_list, player_id);
				if (member_element) 
				{
					sl_list_insert_element(order_data->member_list, member_element);
					ret= TRUE;
				}
			}
			else
			{
				ret= TRUE;
			}
		}
		else
		{
			order_element= order_new(order_list, order_index);
			if (order_element)
			{
				sl_list_insert_element(order_list, order_element);
				
				{
					struct order_list_data *order_data= (struct order_list_data *)order_element->data;
					struct sl_list_element *member_element= order_member_new(order_data->member_list, player_id);

					if (member_element)
					{
						sl_list_insert_element(order_data->member_list, member_element);
						ret= TRUE;
					}
					else
					{
						order_dispose(order_list, order_element);
					}
				}
			}
		}
	}
	else
	{
		ret= TRUE;
	}

	return ret;
}

static void remove_player_from_order(
	short order_index,
	unsigned long player_id)
{
	if (order_index)
	{
		struct sl_list_element *order_element;

		order_element= sl_list_search_for_element(order_list, &order_index);
		if (order_element)
		{
			struct order_list_data *order_data= (struct order_list_data *)order_element->data;
			struct sl_list_element *member_element= sl_list_search_for_element(order_data->member_list, &player_id);

			if (member_element)
			{
				order_member_dispose(order_data->member_list, member_element);
			}
		}
	}
}

static boolean order_list_new(
	void)
{
	boolean ret= FALSE;

	order_list= sl_list_new("order list", order_list_comp_func);
	if (order_list)
	{
		ret= TRUE;
	}

	return ret;
}

static struct sl_list_element *order_new(
	struct sl_list *order_list, 
	short order_index)
{
	struct sl_list_element *order_element= NULL;
	struct order_list_data *order_data;

	if(!order_list)
		return FALSE;
	
	order_data= malloc(sizeof(struct order_list_data));
	if (order_data)
	{
		short *key= malloc(sizeof(short));
		
		if (key)
		{
			char buffer[MAX_SL_LIST_NAME_LENGTH+1];

			*key= order_index;
			sprintf(buffer, "member_list #%d", order_index);
			order_data->order_index= order_index;
			order_data->member_list= sl_list_new(buffer, order_member_list_comp_func);
			if (order_data->member_list)
			{
				order_element= sl_list_new_element(order_list, order_data, key);
				if (order_element)
				{
					// ok
				}
				else
				{
					order_member_list_dispose(order_data->member_list);
					free(key);
					free(order_data);
				}
			}
			else
			{
				free(key);
				free(order_data);
			}
		}
		else
		{
			free(order_data);
		}
	}

	return order_element;
}

static void order_dispose(
	struct sl_list *order_list,
	struct sl_list_element *order_element)
{
	void * data;
	void * key;

	if(!order_list)
		return;
	if(!order_element)
		return;
	sl_list_remove_element(order_list, order_element);
	data= order_element->data;
	key= order_element->key;

	sl_list_dispose_element(order_list, order_element);
	free(data);
	free(key);
}

static struct sl_list_element *order_member_new(
	struct sl_list *member_list, 
	unsigned long player_id)
{
	struct sl_list_element *member_element= NULL;
	struct order_member_list_data *member_data;

	if(!member_list)
		return NULL;
	
	member_data= malloc(sizeof(struct order_member_list_data));
	if (member_data)
	{
		unsigned long *key= malloc(sizeof(unsigned long));

		member_data->player_id= player_id;		
		if (key)
		{
			*key= player_id;
			member_element= sl_list_new_element(member_list, member_data, key);
			if (member_element)
			{
				// ok
			}
			else
			{
				free(key);
				free(member_data);
			}
		}
		else
		{
			free(member_data);
		}
	}

	return member_element;
}

static void order_member_dispose(
	struct sl_list *member_list,
	struct sl_list_element *member_element)
{
	void *data;
	void *key;

	if(!member_list)
		return;
	if(!member_element)
		return;

	sl_list_remove_element(member_list, member_element);
	data= member_element->data;
	key= member_element->key;

	sl_list_dispose_element(member_list, member_element);
	free(data);
	free(key);
}

void order_member_list_dispose(
	struct sl_list *member_list)
{
	sl_list_dispose(member_list);
}

static int order_list_comp_func(
	void *k0,
	void *k1)
{
	short *oi0, *oi1;

	oi0= (short *)k0; oi1= (short *)k1;

	return *oi0-*oi1;
}

static int order_member_list_comp_func(
	void *k0,
	void *k1)
{
	unsigned long *pi0, *pi1;

	pi0= (unsigned long *)k0; pi1= (unsigned long *)k1;

	return *pi0-*pi1;
}

static boolean add_entry_to_login_tree(
	struct bungie_net_player_datum * player,
	unsigned long fpos)
{
	struct element * element;
	struct bungie_net_login_tree_data * data;
	boolean success = FALSE;

	data = malloc(sizeof(struct bungie_net_login_tree_data));
	if (data)
	{
		strcpy(data->login, player->login);
		data->fpos = fpos;
		data->online_data_index = player->player_id - 1;
		element = malloc(sizeof(struct element));
		if (element)
		{
			memset(element, 0, sizeof(struct element));
			element->data = data;
			element->key = (void *)data->login;
			insert_element(&bungie_net_login_tree, element);
			success = TRUE;
		}
	}

	return success;
}

static boolean lock_file_region(
	long offset, 
	long length,
	boolean write,
	boolean block)
{
	struct flock lock_region; 
	int result = -1;
	int lock_type;

	lock_region.l_type = write ? F_WRLCK : F_RDLCK;
	lock_region.l_whence = SEEK_SET;
	lock_region.l_start = offset;
	lock_region.l_len = length;

	lock_type = block ? F_SETLKW : F_SETLK;

	result= fcntl(fd_user_db, lock_type, &lock_region);

	return (result != -1); 
}

static boolean unlock_file_region(
	long offset,
	long length)
{
	int result = -1;
	struct flock unlock_region; 

	unlock_region.l_type = F_UNLCK;
	unlock_region.l_whence = SEEK_SET;
	unlock_region.l_start = offset;
	unlock_region.l_len = length;
	
	result = fcntl(fd_user_db, F_SETLKW, &unlock_region);

	return (result != -1); 
}

static int login_tree_comp_func(
	void * k0,
	void * k1)
{
	char * s0;
	char * s1;

	char buf_0[MAXIMUM_LOGIN_LENGTH + 1];
	char buf_1[MAXIMUM_LOGIN_LENGTH + 1];

	if (k0 && !k1) return -1;
	if (!k0 && k1) return  1;


	s0 = (char *)k0;
	s1 = (char *)k1;

	strcpy(buf_0, s0);
	strcpy(buf_1, s1);

	strlwr(buf_0);
	strlwr(buf_1);

	return strcmp(buf_0, buf_1);
}

static void make_packed_player_data(
	char * player_data,
	short * length,
	struct bungie_net_player_datum * player)
{
	char * p;

	if(!player_data)
		return;
	if(!length)
		return;

	p = player_data;
	*p = player->icon_index;		// coat_of_arms_bitmap_index
	p++;
	*p = player->ranked_score.rank;
	p++;
	*(short *)p = 0;
	p += sizeof(short);				// state;
	*(word *)p = SWAP2(player->primary_color.red);
	p += sizeof(word);
	*(word *)p = SWAP2(player->primary_color.green);
	p += sizeof(word);
	*(word *)p = SWAP2(player->primary_color.blue);
	p += sizeof(word);
	*(word *)p = SWAP2(player->primary_color.flags);
	p += sizeof(word);
	*(word *)p = SWAP2(player->secondary_color.red);
	p += sizeof(word);
	*(word *)p = SWAP2(player->secondary_color.green);
	p += sizeof(word);
	*(word *)p = SWAP2(player->secondary_color.blue);
	p += sizeof(word);
	*(word *)p = SWAP2(player->secondary_color.flags);
	p += sizeof(word);
	*p = player->order_index;		// order icon index
	p++; p++;
	p += sizeof(struct rgb_color); p += sizeof(struct rgb_color);	// order colors
	strcpy(p, player->name);
	p += strlen(player->name) + 1;
	strcpy(p, player->team_name);
	p += strlen(player->team_name) + 1;

	*length = p - player_data;	
}

static int last_response;
static void refresh_response_list(
	void)
{
	unsigned long i;
	last_response = 0;

	for (i = 0; i < MAXIMUM_PLAYER_SEARCH_RESPONSES; i++)
	{
		response_list[i].match_score = 0;
	}
}

static void add_player_to_response_list(
	struct bungie_net_online_player_data * opd)
{
	if (last_response<MAXIMUM_PLAYER_SEARCH_RESPONSES)
	{
		memcpy(&response_list[last_response].aux_data, &opd->aux_data, sizeof(struct metaserver_player_aux_data));
		memcpy(&response_list[last_response].player_data, &opd->player_data, MAXIMUM_PACKED_PLAYER_DATA_LENGTH);
		response_list[last_response].match_score++;
		last_response++;
	}
}

static char * ci_pn_match(
	char * s0,
	char * s1)
{
	static char buf0[MAXIMUM_PLAYER_NAME_LENGTH + 1];
	static char buf1[MAXIMUM_PLAYER_NAME_LENGTH + 1];

	//---assert(s0);
	if(!s0 || !s1)
		return NULL;

	strcpy(buf0, s0);
	strcpy(buf1, s1);

	strlwr(buf0);
	strlwr(buf1);

	return (char *)strstr(buf0, buf1);
};

boolean set_user_game_data(
	unsigned long user_index,
	short game_type,
	struct player_stats * stats)
{
	struct bungie_net_player_datum player;
	boolean success;

	if (get_player_information(NULL, user_index, &player))
	{
		player.ranked_score.damage_inflicted	= stats->points_killed;
		player.ranked_score.damage_received		= player.ranked_score.damage_received;
		player.ranked_score.games_played		= stats->games_played;
		player.ranked_score.wins				= stats->first_place_wins;
		player.ranked_score.losses				= stats->last_place_wins;
		player.room_id							= stats->default_room;
		player.ranked_score.points				= stats->score;
		
		if (update_player_information(NULL, user_index, online_player_data[user_index - 1].logged_in_flag, &player))
		{
			success = TRUE;
		}
	}

	return success;
}	

boolean get_user_game_data(
	unsigned long player_id,
	short game_index,
	struct player_stats * stats,
	word * flags)
{
	struct bungie_net_player_datum player;
	boolean success = FALSE;
	
	if (get_player_information(NULL, player_id, &player))
	{
		stats->points_killed					= player.ranked_score.damage_inflicted;
		stats->points_lost						= player.ranked_score.damage_received;
		stats->units_killed						= 0;
		stats->units_lost						= 0;
		stats->updates_since_last_game_played	= 0;
		stats->games_played						= player.ranked_score.games_played;
		stats->first_place_wins					= player.ranked_score.wins;
		stats->last_place_wins					= player.ranked_score.losses;
		stats->caste							= player.ranked_score.rank;
		stats->default_room						= player.room_id;
		stats->score							= player.ranked_score.points;
		success = TRUE;
	}


	return success;
}

boolean get_user_name(
	long user_index, 
	char * name)
{
	boolean success = FALSE;

	if (user_index <= total_players)
	{
		strcpy(name, online_player_data[user_index - 1].name);
		success = TRUE;
	}

	return success;
}

boolean set_myth_user_data(
	unsigned long user_index,
	char * buffer,
	short length)
{
	boolean success = FALSE;
	char * p;
	struct bungie_net_player_datum player;

	if (get_player_information(NULL, user_index, &player))
	{
		p = buffer;
		player.icon_index = (short)*p;
		p++; p++;
		p += sizeof(short);
		player.primary_color.red = SWAP2(*(word *)p);
		p += sizeof(word);
		player.primary_color.green = SWAP2(*(word *)p);
		p += sizeof(word);
		player.primary_color.blue = SWAP2(*(word *)p);
		p += sizeof(word);
		player.primary_color.flags = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.red = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.green = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.blue = SWAP2(*(word *)p);
		p += sizeof(word);
		player.secondary_color.flags = SWAP2(*(word *)p);
		p += sizeof(word);
		p++; p++;
		p += sizeof(struct rgb_color); p += sizeof(struct rgb_color);
		strcpy(player.name, p);
		p += strlen(player.name) + 1;
		strcpy(player.team_name, p);
		p += strlen(player.team_name) + 1;

		if((p - buffer) != length)
			return FALSE;

		if (update_player_information(NULL, user_index, online_player_data[user_index - 1].logged_in_flag, &player))
		{
			success = TRUE;
		}
	}

	return success;
}

boolean get_myth_user_data(
	unsigned long user_index,
	char *buffer,
	short *length)
{
	boolean success = FALSE;

	if (user_index <= total_players)
	{
		memcpy(buffer, online_player_data[user_index - 1].player_data, MAXIMUM_PACKED_PLAYER_DATA_LENGTH);
		*length = online_player_data[user_index - 1].aux_data.player_data_length;
		success = TRUE;
	}

	return success;
}

void * build_rank_list(
	short game_type, 
	short maximum_users, 
	short *actual_users)
{
	*actual_users = get_user_count();

	return NULL;
}

boolean set_user_as_bungie_admin(
	long user_index, 
	boolean set)
{
	return FALSE;
}

boolean set_user_password(
	long user_index, 
	char * password)
{
	return FALSE;
}


