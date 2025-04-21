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
#include "metaserver_common_structs.h"
#include "sl_list.h"
#include "authentication.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "network_queues.h"
#include "game_search_packets.h"
#include "games_list.h"
#include "../sqlite_utils.h"
#include "matchmaking_sql.h"
#include <sqlite3.h>

static sqlite3 *db = NULL;

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

// ALAN Begin: added headers
#include <string.h>
#include <unistd.h>
// ALAN End

#define	MAXIMUM_OUTSTANDING_REQUESTS	32
#define	SELECT_TIMEOUT_PERIOD		10
#define	MAX_ROOM_ID			128
#define	MAXIMUM_PACKET_SIZE		16*KILO
#define	CLIENT_QUEUE_SIZE		64*KILO

struct
{
	int server_socket;

	fd_set fds_read, fds_write;
	int read_set_size;

	struct sl_list *client_list;
}
server_globals;

struct client_data
{
	int socket;

	struct circular_queue incoming;
	struct circular_queue outgoing;
};

static boolean read_data_into_circular_queue(int fd, int bytes_to_read, struct circular_queue *queue);
static void handle_client_connection(int fd);
static boolean handle_client_data(struct client_data *client);
static boolean handle_game_search_packet(struct room_packet_header *header, struct client_data *client);
static boolean handle_gs_update_packet(struct room_packet_header *header);
static boolean handle_gs_login_packet(struct room_packet_header *header);
static boolean handle_gs_query_packet(struct room_packet_header *header, struct client_data *client);
static void handle_incoming_connection(void);
static void handle_write_data(int fd);
static void run_server(void);
static void init_server(void);
static void transform_update_packet_to_gamedata(struct gs_update_packet *packet, boolean new_game, struct gamedata *gd);
static struct client_data *find_client_from_socket(int socket);
static int client_comp_func(void *k0, void *k1);
static void add_packet_to_outgoing(char *buffer, short length, struct client_data *client);
static boolean send_outgoing_data(struct client_data *client);

int main(
	 int argc,
	 char *argv[])
{
    // Initialize SQLite DB and tables
    if (!initialize_all_tables(&db)) {
        fprintf(stderr, "Failed to initialize SQL database.\n");
        return 1;
    }

	init_server();
	run_server();

	return -1;
}

static void init_server(
	void)
{
	int reuse_address= TRUE;

	server_globals.server_socket= socket(PF_INET, SOCK_STREAM, 0);
	if (server_globals.server_socket!=NONE)
	{
		if (!setsockopt(server_globals.server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(int)))
		{
			struct sockaddr_in address;

			address.sin_family= AF_INET;
			address.sin_addr.s_addr= htonl(INADDR_ANY);
			address.sin_port= htons(DEFAULT_GAME_SEARCH_PORT);

			if (bind(server_globals.server_socket, (struct sockaddr *)&address, sizeof(struct sockaddr_in))!=NONE)
			{
				if (!listen(server_globals.server_socket, MAXIMUM_OUTSTANDING_REQUESTS))
				{
					FD_ZERO(&server_globals.fds_write);
					FD_ZERO(&server_globals.fds_read);
					FD_SET(server_globals.server_socket, &server_globals.fds_read);
					server_globals.read_set_size= server_globals.server_socket+1;

					server_globals.client_list= sl_list_new("client list", client_comp_func);
					if (server_globals.client_list)
					{
						games_list_initialize();
					}
					else
					{
						vhalt("could not allocate client list?");
					}
				}
				else
				{
					vhalt("could not listen on server socket?");
				}
			}
			else
			{
				vhalt("could not bind server socket, resource limit exceeded?");
			}
		}
		else
		{
			vhalt("could not configure server socket as reusable, improper version of sockets?");
		}
	}	
	else
	{
		vhalt("could not create server socket, file max resource limit exceeded?");
	}
}

static void run_server(
	void)
{
	while (1)
	{
		struct timeval timeout;
		fd_set fds_test_read, fds_test_write;
		int result;

		fds_test_read= server_globals.fds_read;
		fds_test_write= server_globals.fds_write;

		timeout.tv_sec= SELECT_TIMEOUT_PERIOD;
		timeout.tv_usec= 0;

		result= select(server_globals.read_set_size, &fds_test_read, NULL, NULL, &timeout);
		if (result==NONE)
		{
			vhalt("system error occurred, select return -1?");
		}
		else
		{
			int fd;
			
			for (fd= 0; fd<server_globals.read_set_size; ++fd)
			{
				if (FD_ISSET(fd, &fds_test_read))
				{
					if (fd==server_globals.server_socket)
						handle_incoming_connection();
					else
						handle_client_connection(fd);

					handle_write_data(fd);
				}
			}			
		}
	}
}

static int client_comp_func(
	void *k0,
	void *k1)
{
	struct client_data *client0, *client1;
	
	client0= (struct client_data *)k0; client1= (struct client_data *)k1;

	return client0->socket-client1->socket;
}

static boolean add_client(
	int socket)
{
	struct client_data *client;
	boolean success= FALSE;

	client= malloc(sizeof(struct client_data));
	if (client)
	{
		memset(client, 0, sizeof(struct client_data));
		client->socket= socket;
		if (allocate_circular_queue("incoming", &client->incoming, CLIENT_QUEUE_SIZE))
		{
			if (allocate_circular_queue("outgoing", &client->outgoing, CLIENT_QUEUE_SIZE))
			{
				struct sl_list_element *element;

				element= sl_list_new_element(server_globals.client_list, client, client);
				if (element)
				{
					sl_list_insert_element(server_globals.client_list, element);
					success= TRUE;
				}
				else
				{
					vpause("could not create client list element? client not added ...");
					free_circular_queue(&client->incoming);
					free_circular_queue(&client->outgoing);
					free(client);
				}
			}
			else
			{
				vpause("could not allocate outgoing circular queue for client? client not added ...");
				free_circular_queue(&client->incoming);
				free(client);
			}
		}
		else
		{
			vpause("could not allocate circular queue for client? client not added ...");
			free(client);
		}
	}
	else
	{
		vpause("could not allocate memory for client, system memory exceeded? client not added ...");
	}

	return success;
}

static void delete_client(
	struct client_data *client)
{
	struct sl_list_element *element;

	FD_CLR(client->socket, &server_globals.fds_read);
	FD_CLR(client->socket, &server_globals.fds_write);
	close(client->socket);

	element= sl_list_search_for_element(server_globals.client_list, client);
	if (element)
	{
		sl_list_remove_element(server_globals.client_list, element);

		free_circular_queue(&client->incoming);
		free_circular_queue(&client->outgoing);
		free(element->data);

		sl_list_dispose_element(server_globals.client_list, element);
	}
}

static void handle_incoming_connection(
	void)
{
	struct sockaddr_in remote_address;	
	int client_socket, client_len;

	client_len= sizeof(struct sockaddr_in);
	client_socket= accept(server_globals.server_socket, (struct sockaddr *)&remote_address, &client_len);
	if (client_socket!=NONE)
	{
		if (add_client(client_socket))
		{
			int flags= fcntl(client_socket, F_GETFL, 0);

			fcntl(client_socket, F_SETFL, O_NONBLOCK|flags);
			FD_SET(client_socket, &server_globals.fds_read);
			FD_SET(client_socket, &server_globals.fds_write);
			server_globals.read_set_size= MAX(server_globals.read_set_size, client_socket+1);
		}
		else
		{
			close(client_socket);
		}
	}
}

static void transform_update_packet_to_gamedata(
	struct gs_update_packet *packet, 
	boolean new_game, 
	struct gamedata *gd)
{
	char *p= (char *)packet;

	memset(gd, 0, sizeof(struct gamedata));
	gd->room_id= packet->room_id;
	gd->game_id= packet->aux_data.game_id;
	gd->aux_data= packet->aux_data;
	gd->description= packet->game;

	if (packet->game.parameters.option_flags&_game_option_allow_veterans_flag) gd->flags|= _game_option_allow_veterans_flag;
	if (packet->game.parameters.option_flags&_game_option_allow_unit_trading_flag) gd->flags|= _game_option_allow_unit_trading_flag;
	if (packet->game.parameters.option_flags&_game_option_allow_multiplayer_teams_flag) gd->flags|= _game_option_allow_multiplayer_teams_flag;
	if (packet->game.parameters.option_flags&_game_option_allow_alliances_flag) gd->flags|= _game_option_allow_alliances_flag;
	if (packet->game.parameters.option_flags&_game_option_allow_overhead_map_flag) gd->flags|= _game_option_allow_overhead_map_flag;
	
	p+= sizeof(struct gs_update_packet);
	strcpy(gd->game_name, p);

	p+= strlen(gd->game_name)+1;
	strcpy(gd->map_name, p);
}

static boolean handle_gs_login_packet(
	struct room_packet_header *header)
{
	struct gs_login_packet *packet;
	char *p= (char *)header;
	boolean disconnect= FALSE;

	p +=sizeof(struct room_packet_header);
	packet= (struct gs_login_packet *)p;

	if (packet->room_id>MAX_ROOM_ID || packet->room_id<0) disconnect= TRUE;

	return disconnect;
}

static boolean handle_gs_update_packet(
	struct room_packet_header *header)
{
	struct gs_update_packet *packet;
	char *p= (char *)header;
	boolean disconnect= FALSE;
	struct gamedata gd;

	p+= sizeof(struct room_packet_header);
	packet= (struct gs_update_packet *)p;

	switch (packet->type_of_update)
	{
	case _add_new_game:
		transform_update_packet_to_gamedata(packet, TRUE, &gd);
		games_list_add_entry(&gd);
		break;

	case _change_game_info:
		transform_update_packet_to_gamedata(packet, FALSE, &gd);
		games_list_add_entry(&gd);
		break;

	case _remove_game:
		games_list_remove_entry(packet->room_id, &packet->aux_data);
		break;

	default:
		vpause(csprintf(temporary, "unknown type of game update packet %d?", packet->type_of_update));
		disconnect= TRUE;
		
	}

	return disconnect;
}

static boolean handle_gs_query_packet(
	struct room_packet_header *header,
	struct client_data *client)
{
	struct gs_query_packet *packet;
	char *p;
	struct gamedata *data;
	boolean ret= FALSE;

	struct games_list_query query;

	p= (char *)header;
	p+= sizeof(struct room_packet_header);
	packet= (struct gs_query_packet *)p;
	p+= sizeof(struct gs_query_packet);

	memset(&query, 0, sizeof(struct games_list_query));
	query.game_scoring= packet->game_scoring;
	query.unit_trading= packet->unit_trading;
	query.veterans= packet->veterans;
	query.teams= packet->teams;
	query.alliances= packet->alliances;
	query.enemy_visibility= packet->enemy_visibility;

	strcpy(query.game_name, p);
	p+= strlen(query.game_name)+1;
	strcpy(query.map_name, p);
	
	data= games_list_search_for_first_match(&query);
	if (data)
	{
		int n= 0;
		char buffer[MAXIMUM_PACKET_SIZE];
		short length;
		struct query_response qr[MAXIMUM_GAME_SEARCH_RESPONSES+1];

		do
		{
			memset(&qr[n], 0, sizeof(struct query_response));
			qr[n].room_id= data->room_id;
			qr[n].aux_data= data->aux_data;
			qr[n].game= data->description;
			qr[n].game_data_length= data->aux_data.game_data_size;
			strcpy(qr[n].game_name, data->game_name);
			strcpy(qr[n].map_name, data->map_name);

			++n;
		} while ((data= games_list_search_for_next_match()) && (n<=MAXIMUM_GAME_SEARCH_RESPONSES));

		qr[n].data_index= DATA_INDEX_UNUSED;

		length= build_gs_query_response_packet(buffer, packet->player_id, &qr[0]);
		add_packet_to_outgoing(buffer, length, client);
	}

	return ret;
}

static boolean handle_game_search_packet(
	struct room_packet_header *header,
	struct client_data *client)
{
	boolean disconnect= FALSE;

	switch (header->type)
	{
	case _gs_login_packet:
		disconnect= handle_gs_login_packet(header);
		break;

	case _gs_update_packet:
		disconnect= handle_gs_update_packet(header);
		break;

	case _gs_query_packet:
		disconnect= handle_gs_query_packet(header, client);
		break;

	default:
		vhalt(csprintf(temporary, "received unknown packet type %d, length %d?", header->type, header->length));
	}

	return disconnect;
}

static boolean handle_client_data(
	struct client_data *client)
{
	char buffer[MAXIMUM_PACKET_SIZE];
	struct room_packet_header *header;
	boolean disconn_from_error= FALSE, disconnect= FALSE;

	header= (struct room_packet_header *)buffer;
	while (!disconnect && get_next_room_packet_from_queue(&client->incoming, header, sizeof(buffer), TRUE, &disconn_from_error))
	{
		byte_swap_game_search_packet((char *)header, FALSE);
		disconnect= handle_game_search_packet(header, client);
	}

	return disconnect;
}

static void handle_client_connection(
	int fd)
{
	struct client_data *client;

	client= find_client_from_socket(fd);
	if (client)
	{
		int nread;
		boolean remove_client= FALSE;
		
		ioctl(fd, FIONREAD, &nread);
		if (!nread) 
		{
			remove_client= TRUE;
		}
		else
		{
			if (read_data_into_circular_queue(fd, nread, &client->incoming))
			{
				remove_client= handle_client_data(client);
			}			
			else
			{
				remove_client= TRUE;
			}
		}

		if (remove_client)
		{
			delete_client(client);
		}
	}
}

static void handle_write_data(
	int fd)
{
	struct client_data *client;

	client= find_client_from_socket(fd);
	if (client && !send_outgoing_data(client)) 
	{
		delete_client(client);
	}
}

static boolean send_outgoing_data(
	struct client_data *client)
{
	boolean ret= TRUE;
	
	do
	{
		long size_to_write;
	
		size_to_write= circular_buffer_linear_write_size(&client->outgoing);
		if (size_to_write)
		{
			long length_sent;

			length_sent= send(client->socket, client->outgoing.buffer+client->outgoing.read_index, size_to_write, 0);
			if (length_sent!=-1)
			{
				if (length_sent!=size_to_write)
				{
					break;
				}
				else
				{
					client->outgoing.read_index+= length_sent;
					if (client->outgoing.read_index>=client->outgoing.size) 
					{
						client->outgoing.read_index= 0;					
						size_to_write= circular_buffer_linear_write_size(&client->outgoing);
						if (!size_to_write) break;
					}
				}
			}
			else
			{
				ret= FALSE;
			}
		}
		else
		{
			break;
		}
	} while (ret);

	return ret;
}

static void add_packet_to_outgoing(
	char *buffer,
	short length,
	struct client_data *client)
{
	byte_swap_game_search_packet(buffer, TRUE);
	if (!copy_data_into_circular_queue(buffer, length, &client->outgoing))
		vpause("could not copy data into circular queue, queue not allocated or blown?");
	byte_swap_game_search_packet(buffer, FALSE);
}

static boolean read_data_into_circular_queue(
	int fd,
	int bytes_to_read,
	struct circular_queue *queue)
{
	char buffer[128];
	int bytes_left= bytes_to_read;
	boolean success= TRUE;

	while (bytes_left)
	{
		int bytes_to_copy= MIN(bytes_left, sizeof(buffer));
		int bytes_read;

		bytes_read= read(fd, buffer, bytes_to_copy);
		if (bytes_read!=NONE)
		{
			if (copy_data_into_circular_queue(buffer, bytes_read, queue))
			{
				// ok
			}
			else
			{
				success= FALSE;
				break;
			}
		}
		else
		{
			success= FALSE;
			break;
		}

		bytes_left-= bytes_read;
	}

	return success;
}

static struct client_data *find_client_from_socket(
	int socket)
{
	struct sl_list_element *element;
	struct client_data *client= NULL;

	element= sl_list_search_for_element(server_globals.client_list, &socket);
	if (element) client= (struct client_data *)element->data;

	return client;	
}
