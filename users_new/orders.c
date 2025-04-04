/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include "environment.h"
#include "metaserver_common_structs.h"
#include "rb_tree.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "orders.h"

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define	BUNGIE_NET_ORDER_DB_SIGNATURE		'ORDR'
#define	UNUSED_ORDER_ID						0xFFFFFFFF

struct bungie_net_order_db_header
{
	unsigned long order_count;
	unsigned long unused[40];
};

struct bungie_net_order_db_entry
{
	unsigned long signature;
	struct bungie_net_order_datum order;
};

struct bungie_net_order_name_tree_data
{
	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];
	long fpos;
};

struct bungie_net_order_id_data
{
	long fpos;
};

static int fd_order_db;
static unsigned long total_orders;
static struct rb_tree bungie_net_order_name_tree;
static struct bungie_net_order_id_data * order_id_indexes;

static boolean add_entry_to_order_name_tree(
	struct bungie_net_order_datum * order,
	unsigned long fpos);

static int order_name_tree_comp_func(
	void * k0,
	void * k1);

boolean create_order_database(
	void)
{
	struct bungie_net_order_db_header header;
	char * file_name;
	boolean success = FALSE;

	memset(&header, 0, sizeof(struct bungie_net_order_db_header));
	file_name = get_orders_db_file_name();
	fd_order_db = open(file_name, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fd_order_db != -1)
	{
		if (write(fd_order_db, &header, sizeof(struct bungie_net_order_db_header)) != -1)
		{
			success = TRUE;
		}
		else
		{
			close(fd_order_db);
			fd_order_db = -1;
		}
	}

	return success;
}

boolean initialize_order_database(
	void)
{
	long fpos;
	unsigned long order_count = 0;
	char * file_name;
	boolean success = FALSE;
	struct bungie_net_order_db_header header;
	struct bungie_net_order_db_entry order;
	
	strcpy(bungie_net_order_name_tree.name, "order name tree");
	bungie_net_order_name_tree.root_element = NULL;
	bungie_net_order_name_tree.comp_func = order_name_tree_comp_func;

	file_name = get_orders_db_file_name();
	fd_order_db = open(file_name, O_RDWR, 0);
	if (fd_order_db != -1)
	{
		int length;

		length = read(fd_order_db, &header, sizeof(struct bungie_net_order_db_header));
		fpos = length;
		if (length == sizeof(struct bungie_net_order_db_header))
		{
			int order_index;

			order_count = header.order_count;
			total_orders = order_count;

			order_id_indexes = 
				(struct bungie_net_order_id_data *)malloc(sizeof(struct bungie_net_order_id_data) * total_orders);
			if (order_id_indexes)
			{
				for (order_index = 0; order_index < order_count; order_index++)
				{
					if (lseek(fd_order_db, sizeof(struct bungie_net_order_db_header) + 
						(order_index * sizeof(struct bungie_net_order_db_entry)), SEEK_SET) != -1)
					{
						length = read(fd_order_db, &order, sizeof(struct bungie_net_order_db_entry));
						if (order.signature != BUNGIE_NET_ORDER_DB_SIGNATURE)
						{
							char buffer[128];
							sprintf(buffer, "invalid signature in order database, offset = %d", fpos);
							vhalt(buffer);
						}

						if (!add_entry_to_order_name_tree(&order.order, fpos))
						{
							break;
						}

						order_id_indexes[order_index].fpos = fpos;

						fpos += length;
					}
				}
			}

			if (order_index == order_count)
			{
				success = TRUE;
			}
		}
	}
	
	return success;
}

void shutdown_order_database(
	void)
{
	return;
}

unsigned long get_order_count(
	void)
{
	struct bungie_net_order_db_header header;
	unsigned long order_count = 0;

	if (lseek(fd_order_db, 0, SEEK_SET) != -1)
	{
		if (read(fd_order_db, &header, sizeof(struct bungie_net_order_db_header)) != -1)
		{
			order_count = header.order_count;
		}
	}

	return order_count;
}

static unsigned long search_order_id = -1;
boolean get_first_order_information(
	struct bungie_net_order_datum * order)
{
	search_order_id = 1;

	return get_order_information(NULL, search_order_id, order);
}

boolean get_next_order_information(
	struct bungie_net_order_datum * order)
{
	boolean ret = FALSE;
	++search_order_id;

	if (search_order_id <= get_order_count())
	{
		ret = get_order_information(NULL, search_order_id, order);
	}

	return ret;
}

boolean get_order_information(
	char * order_name,
	unsigned long order_id,
	struct bungie_net_order_datum * order)
{
	int length;
	struct bungie_net_order_db_entry order_entry;
	struct element * element;
	boolean success = FALSE;

	if(!((order_name && !order_id) || (!order_name && order_id)))
		return FALSE;
	if(!order)
		return FALSE;

	if ((order_id > 0) && (order_id <= total_orders))
	{
		if (lseek(fd_order_db, order_id_indexes[order_id - 1].fpos, SEEK_SET) != -1)
		{
			length = read(fd_order_db, &order_entry, sizeof(struct bungie_net_order_db_entry));
			if (length == sizeof(struct bungie_net_order_db_entry) &&
				order_entry.signature == BUNGIE_NET_ORDER_DB_SIGNATURE)
			{
				memcpy(order, &order_entry.order, sizeof(struct bungie_net_order_datum));
				if (order_id)
				{
					// ok
				}
				success = TRUE;
			}
		}
	}
	else if (order_name)
	{
		struct bungie_net_order_name_tree_data * data;

		element = search_for_element(&bungie_net_order_name_tree, order_name);
		if (element)
		{
			data = element->data;
			if (lseek(fd_order_db, data->fpos, SEEK_SET) != -1)
			{
				length = read(fd_order_db, &order_entry, sizeof(struct bungie_net_order_db_entry));
				if (length == sizeof(struct bungie_net_order_db_entry) &&
					order_entry.signature == BUNGIE_NET_ORDER_DB_SIGNATURE)
				{
					memcpy(order, &order_entry.order, sizeof(struct bungie_net_order_datum));
					success = TRUE;
				}
				else
				{
					halt();
				}
			}
		}
	}

	return success;
}

boolean update_order_information(
	char * order_name,
	unsigned long order_id,
	struct bungie_net_order_datum * order)
{
	struct bungie_net_order_db_entry entry;
	struct element * element;
	int length;
	boolean success = FALSE;

	if(!((order_name && !order_id) || (!order_name && order_id)))
		return FALSE;
	if(!order)
		return FALSE;
	
	if (order_id)
	{
		if (lseek(fd_order_db, order_id_indexes[order_id - 1].fpos, SEEK_SET) != -1)
		{
			entry.signature = BUNGIE_NET_ORDER_DB_SIGNATURE;
			entry.order= *order;
			length = write(fd_order_db, &entry, sizeof(struct bungie_net_order_db_entry));
			if (length == sizeof(struct bungie_net_order_db_entry))
			{
				success = TRUE;
			}
		}
	}
	else if (order_name)
	{
		struct bungie_net_order_name_tree_data * data;

		element = search_for_element(&bungie_net_order_name_tree, order_name);
		if (element)
		{
			data = element->data;
			if (lseek(fd_order_db, data->fpos, SEEK_SET) != -1)
			{
				entry.signature = BUNGIE_NET_ORDER_DB_SIGNATURE;
				entry.order = *order;
				length = write(fd_order_db, &entry, sizeof(struct bungie_net_order_db_entry));
				if (length == sizeof(struct bungie_net_order_db_entry))
				{
					success = TRUE;
				}
			}
		}
	}

	return success;
}

boolean new_order(
	struct bungie_net_order_datum * order)
{
	struct bungie_net_order_db_header header;
	struct bungie_net_order_db_entry entry;
	boolean success = FALSE;
	long fpos, order_fpos;

	struct element * element;

	element = search_for_element(&bungie_net_order_name_tree, (void *)order->name);
	if (!element)
	{
		entry.signature = BUNGIE_NET_ORDER_DB_SIGNATURE;	
		fpos = lseek(fd_order_db, 0, SEEK_SET);
		if (fpos != -1)
		{
			if (read(fd_order_db, &header, sizeof(struct bungie_net_order_db_header)) != -1)
			{
				header.order_count++;
				order->order_id = header.order_count;
				entry.order = *order;
				entry.order.founding_date = time(NULL);

				if (lseek(fd_order_db, sizeof(struct bungie_net_order_db_header) + 
					((order->order_id - 1) * sizeof(struct bungie_net_order_db_entry)), SEEK_SET) != -1)
				{
					if (write(fd_order_db, &entry, sizeof(struct bungie_net_order_db_entry)) != -1)
					{
						order_fpos = sizeof(struct bungie_net_order_db_header) + ((order->order_id - 1) * 
							sizeof(struct bungie_net_order_db_entry));
						fpos = lseek(fd_order_db, 0, SEEK_SET);
						if (fpos != -1)
						{
							if (write(fd_order_db, &header, sizeof(struct bungie_net_order_db_header)) != -1)
							{
								struct bungie_net_order_id_data * oid;

								oid = realloc(order_id_indexes, 
									header.order_count * sizeof(struct bungie_net_order_id_data));
								if (oid)
								{
									order_id_indexes= oid;
									total_orders = header.order_count;
									order_id_indexes[order->order_id - 1].fpos = order_fpos;
									add_entry_to_order_name_tree(order, order_fpos);
									success = TRUE;
								}
							}
						}
					}
				}
			}
		}
	}

	return success;
}

void mark_order_as_unused(
	unsigned long order_id)
{
	struct bungie_net_order_datum order;

	if (get_order_information(NULL, order_id, &order))
	{
		order.order_id = UNUSED_ORDER_ID;
		update_order_information(NULL, order_id, &order);
	}
}

static boolean add_entry_to_order_name_tree(
	struct bungie_net_order_datum * order,
	unsigned long fpos)
{
	struct element * element;
	struct bungie_net_order_name_tree_data * data;
	boolean success = FALSE;

	data = malloc(sizeof(struct bungie_net_order_name_tree_data));
	if (data)
	{
		strcpy(data->order_name, order->name);
		data->fpos = fpos;
		element = malloc(sizeof(struct element));
		if (element)
		{
			memset(element, 0, sizeof(struct element));
			element->data = data;
			element->key = (void *)data->order_name;
			insert_element(&bungie_net_order_name_tree, element);
			success = TRUE;
		}
	}

	return success;
}

static int order_name_tree_comp_func(
	void * k0,
	void * k1)
{
	char * s0;
	char * s1;

	char buf_0[MAXIMUM_ORDER_NAME_LENGTH + 1];
	char buf_1[MAXIMUM_ORDER_NAME_LENGTH + 1];

	if(!k0 || !k1)
		return -1;

	s0 = (char *)k0;
	s1 = (char *)k1;

	strcpy(buf_0, s0);
	strcpy(buf_1, s1);

	strlwr(buf_0);
	strlwr(buf_1);

	return strcmp(buf_0, buf_1);
}
