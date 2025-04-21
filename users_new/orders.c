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
#include "metaserver_common_structs.h"
#include "rb_tree.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "orders.h"

#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// ALAN Begin: added headers
#include <string.h>
#include <errno.h>
// ALAN End

// ALAN Begin: fixed multi-character character constant warnings
//#define	BUNGIE_NET_ORDER_DB_SIGNATURE		'ORDR'
#define	BUNGIE_NET_ORDER_DB_SIGNATURE		0x4f524452
// ALAN End

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
    char *file_name;
    FILE *fp;
    struct bungie_net_order_db_header header;
    struct bungie_net_order_db_entry order_entry;
    unsigned long order_count = 0;
    boolean success = FALSE;
    int i;

    file_name = get_orders_db_file_name();
    fp = fopen(file_name, "rb");
    if (!fp) {
        fprintf(stderr, "[ERROR] [%s:%s] couldn't open %s\n", __FILE__, __func__, file_name);
        return FALSE;
    }

    if (fread(&header, sizeof(header), 1, fp) != 1) {
        fprintf(stderr, "[ERROR] [%s:%s] failed to read header (expected %zu bytes, got less)\n", __FILE__, __func__, sizeof(header));
        fclose(fp);
        return FALSE;
    }
    order_count = header.order_count;
    total_orders = order_count;
    printf("[DEBUG] Order Header Read: count=%lu\n", header.order_count);
    fprintf(stderr, "[DEBUG] [%s:%s] order_count = %lu\n", __FILE__, __func__, order_count);

    if (order_count > 1000000) { // Arbitrary sanity limit
        fprintf(stderr, "[ERROR] [%s:%s] invalid order_count: %lu\n", __FILE__, __func__, order_count);
        fclose(fp);
        return FALSE;
    }

    if (order_count == 0) {
        fprintf(stderr, "[INFO] [%s:%s] no orders to load\n", __FILE__, __func__);
        order_id_indexes = NULL;
        fclose(fp);
        return TRUE;
    }

    order_id_indexes = (struct bungie_net_order_id_data *)malloc(sizeof(struct bungie_net_order_id_data) * order_count);
    printf("[DEBUG] order_id_indexes allocated at %p\n", (void*)order_id_indexes);
    if (!order_id_indexes) {
        fprintf(stderr, "[ERROR] [%s:%s] malloc failed for order_id_indexes\n", __FILE__, __func__);
        fclose(fp);
        return FALSE;
    }

    fpos = sizeof(header);
    for (i = 0; i < (int)order_count; i++) {
        printf("[DEBUG] Reading order %d/%lu...\n", i+1, header.order_count);
        if (fread(&order_entry, sizeof(order_entry), 1, fp) != 1) {
            fprintf(stderr, "[ERROR] [%s:%s] failed to read order entry %d\n", __FILE__, __func__, i);
            free(order_id_indexes);
            order_id_indexes = NULL;
            fclose(fp);
            return FALSE;
        }
        if (order_entry.signature != BUNGIE_NET_ORDER_DB_SIGNATURE) {
            fprintf(stderr, "[ERROR] [%s:%s] invalid signature in order entry %d\n", __FILE__, __func__, i);
            free(order_id_indexes);
            order_id_indexes = NULL;
            fclose(fp);
            return FALSE;
        }
        if (!add_entry_to_order_name_tree(&order_entry.order, fpos)) {
            fprintf(stderr, "[ERROR] [%s:%s] add_entry_to_order_name_tree failed at order %d\n", __FILE__, __func__, i);
            free(order_id_indexes);
            order_id_indexes = NULL;
            fclose(fp);
            return FALSE;
        }
        order_id_indexes[i].fpos = fpos;
        fpos += sizeof(order_entry);
    }

    fclose(fp);
    success = TRUE;
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
