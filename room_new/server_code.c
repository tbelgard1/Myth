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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#define SERVER

#include "metaserver_codes.h"
#include "authentication.h"
#include "metaserver_common_structs.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "server_code.h"
#include "room_globals.h"
#include "games.h"
#include "netdb.h"
#include "remote_commands.h"
#include "game_search_packets.h"

// _MYTHDEV Begin
#include "encodePackets.h"
// _MYTHDEV End
 
// ALAN Begin: added headers
#include <time.h>
#include "games_log.h"
// ALAN End

#define	LARGE_BUFFER_SIZE			32768
#define	MEDIUM_BUFFER_SIZE			16384
#define	OUTGOING_BUFFER_SIZE			16384
#define SCRATCH_BUFFER_SIZE			 4096
 
enum {
	_client_is_gagged_bit,
	_client_is_booted_bit,
	_client_ranking_set_bit,
	NUMBER_OF_CLIENT_FLAGS,
	
	_client_is_gagged_flag= FLAG(_client_is_gagged_bit),
	_client_is_booted_flag= FLAG(_client_is_booted_bit),
	_client_ranking_set_flag= FLAG(_client_ranking_set_bit)
};

enum {
	NUMBER_OF_PINGS_BEFORE_DROPPING= 3,
	DELAY_BEFORE_PINGING= (15*MACHINE_TICKS_PER_SECOND),
	MAXIMUM_BANNER_CHUNK_SIZE= 1024,
	MAXIMUM_BAD_LOGIN_ATTEMPTS= 1,
	MAXIMUM_SINGLE_PACKET_SIZE= 1536,
	INCOMING_BUFFER_SIZE= 4*MAXIMUM_SINGLE_PACKET_SIZE,
	DEAF_DELAY_BEFORE_DROPPING= (7*60*MACHINE_TICKS_PER_SECOND)
};

// evil globals
struct client_data *first_client= NULL;
static unsigned long guest_user_base_id= 0x80000000;

/* --------- local prototypes */
// _MYTHDEV Begin
static boolean decryptData( struct packet_header* packet_header, struct client_data * client_data );
static int encryptData( struct client_data *client, char *buffer, short length);
static boolean handle_session_key_packet(struct client_data *client, struct packet_header *header);
// _MYTHDEV End
static boolean handle_room_login(struct client_data *client, struct packet_header *header);
static boolean handle_logout(struct client_data *client, struct packet_header *header);
static boolean handle_room_broadcast_packet(struct client_data *client, struct packet_header *header);
static boolean handle_create_game_packet(struct client_data *client, struct packet_header *header);
static boolean handle_reset_game_packet(struct client_data *client, struct packet_header *header);
static boolean handle_remove_game_packet(struct client_data *client,struct packet_header *header);
static boolean handle_set_player_data(struct client_data *client,struct packet_header *header);
static boolean handle_directed_data_packet(struct client_data *client, struct packet_header *header);
static boolean handle_set_player_mode_packet(struct client_data *client, struct packet_header *header);
static boolean handle_request_full_update_packet(struct client_data *client, struct packet_header *header);
static boolean handle_game_player_list_packet(struct client_data *client, struct packet_header *header);
static boolean handle_game_score_packet(
	struct client_data * client,
	struct packet_header * header);
static boolean handle_start_game_packet(struct client_data *client, struct packet_header *header);
static boolean handle_game_search_query_packet(struct client_data * client, struct packet_header * header);
static boolean handle_player_info_query_packet(struct client_data * client, struct packet_header * header);

// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
	static boolean handle_player_search_query_packet(struct client_data * client, struct packet_header * header);
	static boolean handle_order_query_packet(struct client_data * client, struct packet_header * header);
	static boolean handle_buddy_query_packet(struct client_data * client, struct packet_header * header);
	static boolean handle_update_buddy_packet(struct client_data * client, struct packet_header * header);

	static boolean handle_update_player_information_packet(
		struct client_data * client,
		struct packet_header * header);
#endif
// ALAN End

static boolean send_response(struct client_data *client, short response_code);
static void log_user_message(struct client_data *client, char *message);
static boolean add_game_to_room(struct client_data *client, short game_port, short order, char *data, short game_length);
static boolean change_game_in_room(struct client_data *client, short order, char *data, short game_length);
static boolean delete_game_from_room(struct client_data *client);
static boolean send_room_motd(struct client_data *client);

static boolean synchronize_player_list(struct client_data *client, unsigned long player_to_send_to,
	unsigned long player_id_changed, short verb);
static boolean synchronize_game_list(struct client_data *client, unsigned long player_to_send_to,
	unsigned long game_id_changed, short verb);
static boolean send_full_update_to_client(struct client_data *client);

static unsigned long find_guest_user_id(void);
static boolean remove_client_from_room(struct client_data *client);
// ALAN Begin: this function defined but not used
// static boolean send_player_to_url(struct client_data *client, short url_type, char *message, char *url);
// ALAN End
static boolean player_in_room(unsigned long player_id, struct client_data *ignore);
static boolean handle_packet(struct client_data *client, struct packet_header *header);
static boolean add_packet_to_outgoing_buffer(struct client_data *client, char *buffer, short length);

static boolean client_has_screen_name(struct client_data *client, char *name);
static boolean myth_player_data_matchs_screen_name(char *player_data, short player_data_size, char *name);
static char *get_myth_screen_name_from_playerdata(char *player_data, short player_data_size);

static boolean is_room_ranked(void);

static boolean add_new_game(struct client_data * p_client, char * p_data, int n_data_length);
static boolean change_registered_game(struct client_data * p_client, char * p_data, int n_data_length);
static boolean delete_registered_game(struct client_data * p_client);

static boolean update_userd_on_player(
	unsigned long player_id,
	boolean log_in);

boolean is_player_admin(
	struct client_data * client)
{
	return client->player_is_admin;
}

boolean is_account_kiosk(
        struct client_data * client)
{
        return client->account_is_kiosk;
}

/* --------- code */
boolean add_client(
	int socket, 
	long host, 
	short port)
{
	struct client_data *client;
	boolean success= FALSE;
	
	client= (struct client_data *) malloc(sizeof(struct client_data));
	if(client)
	{
		memset(client, 0, sizeof(struct client_data));
		client->socket= socket;
		client->host= host;
		client->port= port;
		client->logged_in= FALSE;
		client->player_mode= _player_mode_listening;
		client->last_ping_ticks= machine_tick_count();
		client->ping_count= 0;
		client->game_id= 0;
		client->in_ps_db= FALSE;
		
		client->caste = NONE;
		
		if(allocate_circular_queue("outgoing", &client->outgoing_buffer, OUTGOING_BUFFER_SIZE))
		{
			if(allocate_circular_queue("incoming", &client->incoming_buffer, INCOMING_BUFFER_SIZE))
			{
				success= TRUE;
			}
			else
			{
			}
		}
		else
		{
		}

		if(success)
		{
			if(!first_client)
			{
				first_client= client;
			} else {
				struct client_data *previous;
				
				previous= first_client;
				while(previous->next != NULL) previous= previous->next;
				previous->next= client;
			}
		} else {
			free_circular_queue(&client->incoming_buffer);
			free_circular_queue(&client->outgoing_buffer);
			free(client);
		}
	}
	
	return success;
}

void build_client_writefds(
	fd_set *write_fds)
{
	struct client_data *client= first_client;
	
	FD_ZERO(write_fds);
	while(client)
	{
		if(circular_buffer_size(&client->outgoing_buffer))
		{
			FD_SET(client->socket, write_fds);
		}
		client= client->next;
	}

	return;
}

void set_client_player_information(
	long player_id,
	struct buddy_entry buddies[MAXIMUM_BUDDIES],
	short order,
	short country_code,
	boolean player_is_admin,
	boolean player_is_bungie_employee,
        boolean account_is_kiosk,
	char * login_name)
{
	struct client_data * client = first_client;

	while (client)
	{
		if (client->player_id == player_id)
		{
			memcpy(&client->buddies[0], &buddies[0], 
				sizeof(struct buddy_entry) * MAXIMUM_BUDDIES);
			client->order = order;
			client->player_is_admin = player_is_admin;
			client->country_code = country_code;
			if (client->player_is_admin)
			{
				client->metaserver_aux_data_flags|= _player_is_bungie_caste_icon_flag;
			}
			client->player_is_bungie_employee = player_is_bungie_employee;
                        client->account_is_kiosk = account_is_kiosk;
			if (client->player_is_bungie_employee)
			{
				client->metaserver_aux_data_flags|= _player_is_administrator_flag;
			}
			strcpy(client->name, login_name);
			if (!client->in_ps_db)
			{
				update_userd_on_player(client->player_id, TRUE);
				update_client_buddy_list(client->player_id, buddies);
				client->in_ps_db = TRUE;
			}
			synchronize_player_list(client, 0, client->player_id, _change_player_verb);

			break;
		}
		client = client->next;
	}
}

boolean set_client_ranking(
	unsigned long player_id, 
	short flags,
	struct player_stats *stats)
{
	struct client_data *client= first_client;

	while(client)
	{
		if(client->player_id==player_id)
		{
			short new_caste= stats->caste;
			
			if (client->player_is_guest)
			{
				flags|= _player_is_anonymous_flag;
			}

			if (client->player_is_admin)
			{
				flags|= _player_is_bungie_caste_icon_flag;
			}

			if (client->player_is_bungie_employee)
			{
				flags|= _player_is_administrator_flag;
			}


			if ((client->caste != new_caste) && client->player_data_set)
			{
				struct metaserver_player_aux_data player_aux_data;
				short length;
				char buffer[SCRATCH_BUFFER_SIZE];

				memset(&player_aux_data, 0, sizeof(struct metaserver_player_aux_data));
				client->caste = (client->player_is_guest || !room_globals.ranked_room) ? NONE : new_caste;
				player_aux_data.verb= _change_player_verb;
				player_aux_data.flags= client->metaserver_aux_data_flags;
				player_aux_data.ranking= client->caste;
				player_aux_data.player_id= client->player_id;
				player_aux_data.caste= (client->player_is_guest || !room_globals.ranked_room) ? NONE : client->caste;
				player_aux_data.player_data_length= client->player_data_size;
				player_aux_data.order= client->order;
		
				length= build_set_player_data_from_metaserver_packet(buffer, &player_aux_data, 
					client->player_data, client->player_data_size);
				if(length<0 || length>=sizeof(buffer))
					return FALSE;
				if (send_message_to_player_in_room(client->player_id, buffer, length) == FALSE)
				{
					return FALSE;
				}
			}

			client->caste = new_caste;
		}
		client= client->next;
	}

	return TRUE;
}

void closedown_client(
	struct client_data *client)
{
	if(client->logged_in)
	{
		log_user_message(client, "logged off...");
		update_userd_on_player(client->player_id, FALSE);
		remove_client_from_room(client);
	}
	
	return;
}

struct client_data * find_client_from_player_id(
	unsigned long ul_player_id)
{
	struct client_data * p_client = first_client;
	
	while (p_client)
	{
		if (p_client->player_id == ul_player_id)
		{
			break;
		}
		p_client = p_client->next;
	}

	return p_client;
}

struct client_data *find_client_from_socket(
	int socket)
{
	struct client_data *client= first_client;
	
	while(client && client->socket != socket) client= client->next;
	
	return client;
}

unsigned long find_client_id_from_screen_name(
	char * name) 
{
	struct client_data *client= first_client;
	unsigned long actual_id= NONE;
	
	while(client && actual_id==NONE) 
	{
		if(client_has_screen_name(client, name))
		{
			actual_id= client->player_id;
		}
		client= client->next;
	}
	
	return actual_id;
}

struct client_data * get_client_from_player_id(
	unsigned long player_id)
{
	struct client_data * client= first_client;
	
	while (client) 
	{
		if(client->player_id==player_id)
		{
			break;
		}
		client= client->next;
	}

	return client;
}

boolean get_client_screen_name(
	unsigned long player_id, 
	char *name)
{
	struct client_data *client= first_client;
	boolean found= FALSE;
	
	while(client && !found) 
	{
		if(client->player_id==player_id)
		{
			switch(room_globals.game_type)
			{
				case 0:
					strcpy(name, get_myth_screen_name_from_playerdata(client->player_data, client->player_data_size));
					break;
					
				default:
					halt();
					break;
			}
			found= TRUE;
		}
		client= client->next;
	}

	if (!found)
	{
		strcpy(name, "<unknown screen name>");
	}
	
	return found;
}

short current_client_count(
	void)
{
	struct client_data *client= first_client;
	short count= 0;
	
	while(client) 
	{
		count+= 1;
		client= client->next;
	}
	
	return count;
}

void delete_client(
	struct client_data *dead)
{
	if(!empty_stats(&dead->delta_stats))
	{
		// they logged off or changed rooms.  Send it to the user server immediately.
		update_userd_with_delta_stats(dead->player_id, &dead->delta_stats);
		memset(&dead->delta_stats, 0, sizeof(struct player_stats));
	}

	if(first_client==dead)
	{
		first_client= dead->next;
	}
	else
	{
		struct client_data *previous= first_client;
		
		while(previous && previous->next != dead)
			previous= previous->next;
		if(previous)
			previous->next= dead->next;
	}
	free_circular_queue(&dead->incoming_buffer);
	free_circular_queue(&dead->outgoing_buffer);
	free(dead);
	
	return;
}

void refuse_client_room_full(	
	int socket)
{
	char buffer[SCRATCH_BUFFER_SIZE];
	short length, length_sent;

	// at this point, we don't have a client
	length= build_server_message_packet(buffer, _room_full_msg);
	if(length>=sizeof(buffer))
		return;

	// byteswap it and send it.
	byteswap_packet(buffer, TRUE);
	length_sent= send(socket, buffer, length, 0);
	// we don't care if we didn't send it all, they're going to be disconnected anyway.
	
	return;
}

// return TRUE if they should be dropped.
boolean ping_client(
	struct client_data *client)
{
	boolean done= FALSE;

	if(client->flags & _client_is_booted_flag)
	{
		done= TRUE;
	}
	else
	{
		long ticks = machine_tick_count();
		
		if(client->deaf)
		{
			if(ticks-client->last_ping_ticks > DEAF_DELAY_BEFORE_DROPPING)
			{
				log_user_message(client, "connection closed by ping timeout (they were deaf and didn't update us)");
				done= TRUE;
			}
		}
		else
		{
			if(ticks-client->last_ping_ticks > DELAY_BEFORE_PINGING)
			{
				if(++client->ping_count > NUMBER_OF_PINGS_BEFORE_DROPPING)
				{
					log_user_message(client, "connection closed by ping timeout");
					done= TRUE;
				}
				else
				{
					char buffer[SCRATCH_BUFFER_SIZE];
					short length;
				
					// ping them...
					length= build_keepalive_packet(buffer);
					if(length>=sizeof(buffer))
						return TRUE;
					done = add_packet_to_outgoing_buffer(client, buffer, length);
				}
				client->last_ping_ticks = ticks;
			}
		}
	}
	
	return done;
}

struct client_data *find_first_client(
	void)
{
	return first_client;
}

int get_client_socket(
	struct client_data *client)
{
	if(client)
		return client->socket;
	else
		return NONE;
}

struct client_data *find_next_client(
	struct client_data *client)
{
	if(client)
		return client->next;
	else
		return NULL;
}

boolean copy_data_into_client_buffer(
	struct client_data *client,
	char *buffer,
	int length)
{
	if(!copy_data_into_circular_queue(buffer, length, &client->incoming_buffer))
	{
		return FALSE;
	}
	
	return TRUE;
}

boolean handle_client_data(	
	struct client_data *client,
	boolean *has_outgoing_data)
{
	boolean disconnect_client= FALSE;
	boolean byteswap= FALSE;
	struct packet_header *header;
	char packet[MAXIMUM_SINGLE_PACKET_SIZE] = {0};
	boolean disconn_from_error=FALSE;
		
#ifdef little_endian
	byteswap= TRUE;
#endif
	
	header= (struct packet_header *) packet;
	while(!disconnect_client && get_next_packet_from_queue(&client->incoming_buffer, header, MAXIMUM_SINGLE_PACKET_SIZE, byteswap, &disconn_from_error))
	{
		// _MYTHDEV Begin
        if ( !decryptData( header, client ) ) { 
            disconnect_client = TRUE;
        } else {   
		// _MYTHDEV End
			if(byteswap_packet((char *) header, FALSE) == TRUE)
			{
				disconnect_client= handle_packet(client, header);
			} else {
				disconnect_client= TRUE;
			}
		} // _MYTHDEV
	}

	if (disconn_from_error == TRUE)
	{
		disconnect_client = TRUE;
	}

	if(!disconnect_client)
	{
		*has_outgoing_data= ((circular_buffer_size(&client->outgoing_buffer)>0) ? TRUE : FALSE);
	}

	return disconnect_client;
}

boolean send_message_to_player_in_room(
	unsigned long destination_player_id, 
	char *message, 
	short length)
{
	struct client_data *client= first_client;
	
	if (length > 0)
	{
		while(client)
		{	
			if(client->logged_in && !client->deaf && (!destination_player_id || client->player_id==destination_player_id))
			{
				if (add_packet_to_outgoing_buffer(client, message, length) == TRUE)
				{
					return FALSE;
				}
			}
			client= client->next;
		}
	}

	return TRUE;
}

boolean send_room_message_to_everyone(
	char *message)
{
	struct client_data *client;
	
	client= first_client;

	if (strlen(message) > 0)
	{
		while(client)
		{
			if (send_room_message_to_player(client, message) == TRUE)
			{
				return FALSE;
			}
			client= client->next;
		}
	}

	return TRUE;	
}

boolean send_room_message_to_player(
	struct client_data *client,
	char *message)
{
	boolean done= FALSE;

	if(!client)
		return FALSE;
		
	if(client->logged_in && !client->deaf && strlen(message) > 0)
	{
		short response_length;
		char buffer[SCRATCH_BUFFER_SIZE];

		response_length= build_message_of_the_data_packet(buffer, message);
		if(response_length<sizeof(buffer))
			done= add_packet_to_outgoing_buffer(client, buffer, response_length);
	}
	
	return done;
}

void list_clients_logged_in(	
	void)
{
	return;
}

short build_room_status_packet(
	char *buffer)
{
	struct client_data *client;
	short players_available_count, players_in_game_count, games_available_count, games_in_progress_count;
	short guest_player_count;
	short length;

	players_available_count= players_in_game_count= guest_player_count= 0;
	games_available_count= games_in_progress_count= 0;
	
	client= first_client;
	while(client)
	{	
		if(client->player_is_guest)
		{
			guest_player_count++;
		} else {
			if(client->deaf)
			{
				players_in_game_count++;
			} else {
				players_available_count++;
			}
		}
		
		if(client->game_id)
		{
			games_available_count++;
			if(client->deaf)
			{
				games_in_progress_count++;
			}
		}
		
		client= client->next;
	}
	
	length= start_building_rs_status_packet(buffer, players_available_count,
		players_in_game_count, games_available_count, games_in_progress_count,
		guest_player_count);

	client= first_client;
	while(client)
	{	
		if(!client->player_is_guest)
		{
			length= add_player_to_rs_status_packet(buffer, client->player_id);
		}
		
		client= client->next;
	}

	return length;
}

short build_short_room_status_packet(
	char *buffer)
{
	struct client_data *client;
	short player_count, game_count;

	player_count= game_count= 0;
	
	client= first_client;
	while(client)
	{	
		player_count++;
		if(client->game_id)
		{
			game_count++;
		}
		
		client= client->next;
	}

	return build_rs_update_room_data_packet(buffer, player_count, game_count);
}

boolean client_is_admin(
	struct client_data *client)
{
	boolean is_admin= FALSE;

	if(!client->player_id)
		is_admin= TRUE;
	
	return is_admin;
}

boolean send_outgoing_data(
	struct client_data *client)
{
	boolean done= FALSE;
	boolean error= FALSE;
	
	do {
		long size_to_write;
		
		size_to_write= circular_buffer_linear_write_size(&client->outgoing_buffer);
		if(size_to_write)
		{
			long length_sent;
		
			length_sent= send(client->socket, client->outgoing_buffer.buffer+client->outgoing_buffer.read_index, size_to_write, 0);
			if(length_sent==-1)
			{
				error= TRUE;
				done= TRUE;
			} else {
				if(length_sent != size_to_write)
				{
					done= TRUE;
				}
				
				client->outgoing_buffer.read_index+= length_sent;
				if(client->outgoing_buffer.read_index>=client->outgoing_buffer.size) client->outgoing_buffer.read_index= 0;
			}
		} else {
			done= TRUE;
		}
	} while(!done);
	
	return error;
}

boolean get_logged_in_player_id_stats(
	unsigned long player_id,
	struct player_stats *stats)
{
	struct client_data *client;
	boolean found= FALSE;
	
	client= first_client;
	while(client && !found)
	{
		if(client->player_id==player_id)
		{
			memcpy(stats, &client->stats, sizeof(struct player_stats));
			add_player_stats(stats, &client->delta_stats, stats);
			found= TRUE;
		}
		client= client->next;
	}
	
	return found;
}

boolean get_player_login_name(
	unsigned long player_id, 
	char *login_name)
{
	struct client_data *client;
	boolean found= FALSE;
	
	client= first_client;
	while(client && !found)
	{
		if(client->player_id==player_id)
		{
			strcpy(login_name, client->name);
			found= TRUE;
		}
		client= client->next;
	}

	if (!found)
	{
		strcpy(login_name, "<unknown login>");
	}
	
	return found;
}

unsigned long get_client_player_id(
	struct client_data *client)
{
	return client->player_id;
}

#define GAMES_PLAYED_BEFORE_UPDATE (1)
void increment_player_transient_stats(
	unsigned long player_id, 
	struct player_stats *s)
{
	struct client_data *client;
	boolean found= FALSE;
	
	client= first_client;
	while(client && !found)
	{
		if(client->player_id==player_id)
		{
			add_player_stats(&client->delta_stats, s, &client->delta_stats);
			
			if(client->delta_stats.games_played>=GAMES_PLAYED_BEFORE_UPDATE)
			{
				update_userd_with_delta_stats(player_id, s);
				memset(&client->delta_stats, 0, sizeof(struct player_stats));
			}
			found= TRUE;
		}
		client= client->next;
	}
	
	if(!found)
	{
		update_userd_with_delta_stats(player_id, s);
	}
	
	return;
}

struct client_data *find_client_by_id(
	unsigned long player_id)
{
	struct client_data *client;
	struct client_data *actual_client= NULL;
	
	client= first_client;
	while(client && !actual_client)
	{
		if(client->player_id==player_id)
		{
			actual_client= client;
		}
		client= client->next;
	}
	
	return actual_client;
}

boolean boot_player(
	struct client_data *client,
        char* message)
{
	if(!client)
		return FALSE;
	
	client->flags |= _client_is_booted_flag;

	if (message)
	{
			char buffer[SCRATCH_BUFFER_SIZE];
			short response_length = build_you_just_got_blammed_sucka_packet(buffer, message);
			if(response_length < sizeof(buffer))
	             add_packet_to_outgoing_buffer(client, buffer, response_length);
			else
				return FALSE;
	}
	
	return TRUE;
}

boolean gag_player(
	struct client_data *client,
	boolean gag)
{
	if(!client)
		return FALSE;
	if(gag)
	{
		client->flags |= _client_is_gagged_flag;
	} else {
		client->flags &= ~_client_is_gagged_flag;
	}
	
	return TRUE;
}

void update_userd_with_delta_stats(
	unsigned long player_id, 
	struct player_stats *a)
{
	char buffer[SCRATCH_BUFFER_SIZE];
	short length;

	print_stats(a);
	length= build_rs_update_ranking_packet(buffer, player_id, a);
	send_room_packet(room_globals.userd_socket, buffer, length);
	
	return;
}

boolean update_client_buddy_list(
	long player_id, 
	struct buddy_entry * buddies)
{
	char buffer[SCRATCH_BUFFER_SIZE];
	short packet_length;

	struct client_data * client;
	boolean success = FALSE;

	client = first_client;
	while (client)
	{
		if (client->player_id == player_id)
		{
			break;
		}

		client = client->next;
	}

	if (client)
	{
		memcpy(client->buddies, buddies, sizeof(struct buddy_entry) * MAXIMUM_BUDDIES);
		packet_length = build_update_player_buddy_list_packet(buffer, buddies);		
		success = !send_message_to_player_in_room(player_id, buffer, packet_length);	
	}

	return success;
}

boolean send_order_update_to_player(
	long player_id,
	short member_count,
	struct order_member * order_members)
{
	char buffer[SCRATCH_BUFFER_SIZE];
	short packet_length;

	packet_length = build_order_member_list_packet(buffer, member_count, order_members);
	return !send_message_to_player_in_room(player_id, buffer, packet_length);
}

boolean send_player_info_packet_to_player(
	struct rs_player_info_reply_packet * packet)
{
	short length;
	char buffer[MEDIUM_BUFFER_SIZE] = {0};

	char * p;
	p = (char *)packet;
	p += sizeof(struct rs_player_info_reply_packet);

	// ALAN Begin: fixed passing args from incompatible pointer types (2)
//	length = build_player_info_packet(buffer, packet->administrator_flag, packet->bungie_employee_flag, packet->order_index, packet->icon_index, 
//		&packet->primary_color, &packet->secondary_color, &packet->unranked_score_datum, &packet->ranked_score_datum, &packet->ranked_score_datum_by_game_type,
//		&packet->order_unranked_score_datum, &packet->order_ranked_score_datum, &packet->order_ranked_score_datum_by_game_type, &room_globals.overall_rank_data, p);
	length = build_player_info_packet(buffer, packet->administrator_flag, packet->bungie_employee_flag, packet->order_index, packet->icon_index, 
		&packet->primary_color, &packet->secondary_color, &packet->unranked_score_datum, &packet->ranked_score_datum, packet->ranked_score_datum_by_game_type,
		&packet->order_unranked_score_datum, &packet->order_ranked_score_datum, packet->order_ranked_score_datum_by_game_type, &room_globals.overall_rank_data, p);
	// ALAN End

	return !send_message_to_player_in_room(packet->player_id, buffer, length);
}

boolean send_room_list_to_clients(
	struct player_room_list_data *available_rooms, 
	short available_room_count)
{
	struct client_data *client= first_client;
	boolean bRet = TRUE;
	
	while(client)
	{
		struct player_room_list_data room_copy[MAXIMUM_ROOMS];
		short room_count, length, index;
		char buffer[SCRATCH_BUFFER_SIZE] = {0};
		
		memset(room_copy, 0, sizeof(struct player_room_list_data)*MAXIMUM_ROOMS);

		// filter the list...		
		if(available_room_count<0 || available_room_count>MAXIMUM_ROOMS)
			return FALSE; // ?
		memcpy(room_copy, available_rooms, available_room_count*sizeof(struct player_room_list_data));
		room_count= find_client_available_rooms(client->caste, client->country_code, client->name,
			room_copy, available_room_count, FALSE);

		// send it.
		length= start_building_room_packet(buffer);
		for(index= 0; index<room_count; ++index)
		{
			length= add_room_data_to_packet(buffer, &room_copy[index].info);
		}

		if (length > 1024)
		{
		}
		if (send_message_to_player_in_room(client->player_id, buffer, length) == FALSE)
		{
			bRet = FALSE;
		}		
		client= client->next;
	}
	
	return bRet;
}

void report_game_to_userd(
	struct game_data * game)
{
	char buffer[LARGE_BUFFER_SIZE];
	short length;

	if (!game->reported)
	{
		game->reported= TRUE;

		output_games_log_message("game scoring info reported to userd", room_globals.room_identifier, game->game_id);

		length = build_rs_score_game_packet(buffer, game);
		send_room_packet(room_globals.userd_socket, buffer, length);
	}
}

char *get_login_name_from_client(
	void *client)
{
	char *ret= "this is a bad string";
	struct client_data *c= (struct client_data *)client;

	if (c) ret= c->name;

	return ret;
}

/* ----------- packet handlers */
// packet is already byteswapped...
static boolean handle_packet(
	struct client_data *client, 
	struct packet_header *header)
{
	boolean done= FALSE;

	switch(header->type)
	{
		case _player_info_query_packet:
			done = handle_player_info_query_packet(client, header);
			break;

        // _MYTHDEV Begin
		case _session_key_packet: 
            done= handle_session_key_packet(client, header);  
			break;
        // _MYTHDEV End

		case _room_login_packet:
			done= handle_room_login(client, header);
			break;
			
		case _logout_packet:
			done= handle_logout(client, header);
			break;
			
		case _set_player_data_packet:
			done= handle_set_player_data(client, header);
			break;

		case _game_search_query_packet:
			done = handle_game_search_query_packet(client, header);
			break;

#ifdef BN2_FULLVERSION
		case _player_search_query_packet:
			done = handle_player_search_query_packet(client, header);
			break;
#elif defined(BN2_DEMOVERSION)
		case _player_search_query_packet:
			done = FALSE;
			break;
#endif

#ifdef BN2_FULLVERSION
		case _order_query_packet:
			done = handle_order_query_packet(client, header);
			break;
#elif defined(BN2_DEMOVERSION)
		case _order_query_packet:
			done = FALSE;
			break;
#endif

#ifdef BN2_FULLVERSION
		case _buddy_query_packet:
			done = handle_buddy_query_packet(client, header);
			break;
#elif defined(BN2_DEMOVERSION)
		case _buddy_query_packet:
			done = FALSE;
			break;
#endif

#ifdef BN2_FULLVERSION
		case _update_buddy_packet:
			done = handle_update_buddy_packet(client, header);
			break;
#elif defined(BN2_DEMOVERSION)
		case _update_buddy_packet:
			done = FALSE;
			break;
#endif

#ifdef BN2_FULLVERSION
		case _update_player_information_packet:
			done = handle_update_player_information_packet(client, header);
			break;
#elif defined(BN2_DEMOVERSION)
		case _update_player_information_packet:
			done = FALSE;
			break;
#endif

		// Sent by both
		case _room_broadcast_packet:
			if (!room_globals.squelched || client->player_is_bungie_employee)
			{
				done= handle_room_broadcast_packet(client, header);
			}
			break;
			
		case _create_game_packet:
			done= handle_create_game_packet(client, header);
			break;

		case _reset_game_packet:
			done= handle_reset_game_packet(client, header);
			break;

		case _remove_game_packet:
			done= handle_remove_game_packet(client, header);
			break;
			
		case _directed_data_packet:
			done= handle_directed_data_packet(client, header);
			break;
			
		case _set_player_mode_packet:
			done= handle_set_player_mode_packet(client, header);
			break;
			
		case _data_chunk_reply_packet:
			done= TRUE; // obsolete
			break;
			
		case _request_full_update_packet:
			done= handle_request_full_update_packet(client, header);
			break;
			
		case _keepalive_packet:
			break;
			
		case _game_player_list_packet:
			done= handle_game_player_list_packet(client, header);
			break;
			
		case _game_score_packet:
			done= handle_game_score_packet(client, header);
			break;
			
		case _start_game_packet:
			done= handle_start_game_packet(client, header);
			break;

		case _room_list_packet:
		case _player_list_packet:
		case _game_list_packet:
		default:
			done = TRUE;
			break;
	}

	client->last_ping_ticks= machine_tick_count();
	client->ping_count= 0;

	return done;
}

// _MYTHDEV Begin
static boolean handle_session_key_packet(  
	struct client_data *client, 
	struct packet_header *header)
{
    char* their_public_key = ((char *) header + sizeof(struct packet_header));  
	boolean done= FALSE;
	short length;
    char my_public_key[ SESSION_KEY_SIZE ];
    char my_private_key[ SESSION_KEY_SIZE ];
    char buffer[SCRATCH_BUFFER_SIZE];

    getPublicKey( my_public_key, my_private_key );
    getSessionKey( client->session_key, my_private_key, their_public_key );

	length= build_sessionkey_packet(buffer, my_public_key , SESSION_KEY_SIZE);
	done= add_packet_to_outgoing_buffer(client, buffer, length);
    client->valid_session_key = TRUE;
	return done;
} 
// _MYTHDEV End

static boolean handle_room_login(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	struct room_login_data *login= (struct room_login_data *) ((byte *) header+sizeof(struct packet_header));
	short response_code= NONE;

	if(!client->logged_in)
	{
		if(guest_token(&login->token))
		{
			client->player_is_guest= TRUE;
			client->stats.caste= NONE;
			client->logged_in= TRUE;
			client->player_id= find_guest_user_id();
			strcpy(client->name, "Guest");
		} 
		else if(authenticate_token(client->host, get_current_time(), &client->player_id, login->token))
		{
			client->player_is_guest= FALSE;
			client->logged_in= TRUE;
		} else {
			log_user_message(client, "had a bad token.");
			if(++client->login_attempts>=MAXIMUM_BAD_LOGIN_ATTEMPTS) done= TRUE;
			response_code= _login_failed_bad_user_or_password;
		}
		
		if(client->logged_in)
		{
			char buffer[SCRATCH_BUFFER_SIZE];
			char message[SCRATCH_BUFFER_SIZE];
			short length;

			strncpy(client->name, login->name, MAXIMUM_METASERVER_USERNAME+1);
			client->name[MAXIMUM_METASERVER_USERNAME]= 0;

			if(player_in_room(client->player_id, client))
			{
				// the same player can't be in the same room more than once.
				response_code= _account_already_logged_in;
				strcpy(message, "already logged into this room and tried to do it again!");

				client->logged_in= FALSE;
			} else {
				length= build_room_login_successful_packet(buffer, client->player_id, room_globals.maximum_players_per_room);
				if(length>sizeof(buffer))
					return FALSE;
				done= add_packet_to_outgoing_buffer(client, buffer, length);
				if(!done)
				{
					response_code= _login_successful_msg;

					client->metaserver_aux_data_flags= 0;
					if(client->player_is_guest)
					{
						strcat(message, " as guest");
						client->metaserver_aux_data_flags |= _player_is_anonymous_flag;
						client->stats.caste= NONE;
					} else {
						length= build_rs_request_ranking_packet(buffer, client->player_id);
						if(length>sizeof(buffer))
							return FALSE;
						if(room_globals.userd_socket!=NONE)
						{
							send_room_packet(room_globals.userd_socket, buffer, length);
						}
					}
				} else {
					strcpy(message, "unable to add data to their outgoing buffer!");
				}
			}
			log_user_message(client, message);
		}
	} else {
		log_user_message(client, "tried to log in twice?");
		response_code= _login_successful_msg;
	}

	if (done == TRUE)
	{
		return TRUE;
	}

	done |= send_response(client, response_code);

	if(response_code==_login_successful_msg)
	{
		done|= send_room_motd(client);
	}

	return done;
}

static boolean handle_logout(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	UNUSED_PARAMETER(header);
	if(client->logged_in)
	{
		response_code= _logout_successful_msg;
		log_user_message(client, "logged out.");
		if (remove_client_from_room(client) == FALSE)
		{
			done = TRUE;
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in! (Tried a logout)");
		done= TRUE;
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_room_broadcast_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		if(!handle_remote_command(client, (char *) header+sizeof(struct packet_header), header->length-sizeof(struct packet_header)))
		{
			if(client->flags & _client_is_gagged_flag)
			{
				if (send_room_message_to_player(client, "You have been gagged. If you feel this was unfair, send email to your@email.com") == FALSE)
				{
					return TRUE;
				}
			} else {
				if (send_message_to_player_in_room(0, (char *) header, header->length) == FALSE)
				{
					return TRUE;
				}
			}
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in! (Tried a room broadcast)");
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_create_game_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		char * p;
		struct create_game_packet * packet;
		char message[128];
		short game_data_length= header->length-(sizeof(struct packet_header) + sizeof(struct create_game_packet));

		if (game_data_length < 512)
		{
			return TRUE;
		}

		p = (char *)header;
		p += sizeof(struct packet_header);

		packet = (struct create_game_packet *)p;
		
		p += sizeof(struct create_game_packet);

		if(game_data_length<0 || game_data_length>=MAXIMUM_METASERVER_GAME_DATA_SIZE)
			return FALSE;
		/* Copy it */
		if(!client->game_id)
		{
			sprintf(message, "created a game on port %d!", packet->port);
			if (add_game_to_room(client, packet->port, packet->order, p, game_data_length) == FALSE)
			{
				return TRUE;
			}
			add_new_game(client, p, game_data_length);
		} else {
			sprintf(message, "changing game data");
			if (change_game_in_room(client, packet->order, p, game_data_length) == FALSE)
			{
				return TRUE;
			}
			change_registered_game(client, p, game_data_length);
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in! (Tried to create a game)");
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_reset_game_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		if(client->game_id==NONE)
		{
			log_user_message(client, "tried to reset a game, but they didn't have one!");
		} else {
			log_user_message(client, "reset game data");
			reset_game_for_new_round(client->game_id);
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in! (Tried a create game)");
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_remove_game_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		if(client->game_id)
		{
			log_user_message(client, "removed their game.");
			if (delete_game_from_room(client) == FALSE)
			{
				return TRUE;
			}
		} else {
			log_user_message(client, "tried to remove their game (and they didn't have one)");
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in! (tried to remove a game).");
	}

	done |= send_response(client, response_code);

	return done;
}

// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
static boolean handle_update_buddy_packet(
	struct client_data * client,
	struct packet_header * header)
{
	struct update_buddy_packet * packet;
	char * p;
	char buffer[SCRATCH_BUFFER_SIZE];
	short packet_length;

	p = (char *)header;
	p += sizeof(struct packet_header);

	packet = (struct update_buddy_packet *)p;
	packet_length = build_rs_update_buddy_packet(buffer, client->player_id, packet->buddy_id, packet->add);
	send_room_packet(room_globals.userd_socket, buffer, packet_length);

	return FALSE;
	
}
#endif
// ALAN End

static boolean handle_set_player_data(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		char *data= (char *) header + sizeof(struct packet_header);
		long player_data_size= header->length-sizeof(struct packet_header);
		short packet_length;
		char buffer[SCRATCH_BUFFER_SIZE];

		if (player_data_size>=0 && player_data_size<=MAXIMUM_METASERVER_PLAYER_DATA_SIZE)
		{
			memcpy(client->player_data, data, player_data_size);
			client->player_data_size= player_data_size;

			packet_length = build_rs_player_information_query_packet(buffer, client->player_id);
			send_room_packet(room_globals.userd_socket, buffer, packet_length);

			if(!client->player_data_set)
			{
				struct client_data *c= first_client;
				struct metaserver_player_aux_data player_aux_data;

				while(c)
				{
					if(c->player_id != client->player_id)
					{
						if (synchronize_player_list(client, c->player_id, client->player_id, _add_player_verb) == FALSE)
						{
							return TRUE;
						}
					}
					c= c->next;
				}

				if (synchronize_player_list(client, client->player_id, 0, _add_player_verb) == FALSE)
				{
					return TRUE;
				}
			
				if (synchronize_game_list(client, client->player_id, 0, _add_game_verb) == FALSE)
				{
					return TRUE;
				}

				memset(&player_aux_data, 0, sizeof(struct metaserver_player_aux_data));
				player_aux_data.verb= _change_player_verb;
				player_aux_data.flags= client->metaserver_aux_data_flags;
				player_aux_data.ranking= client->caste;
				player_aux_data.player_id= client->player_id;
				player_aux_data.caste= (client->player_is_guest || !room_globals.ranked_room) ? NONE : client->caste;
				player_aux_data.player_data_length= client->player_data_size;
				player_aux_data.order= client->order;
				packet_length= build_set_player_data_from_metaserver_packet(buffer, &player_aux_data, 
					client->player_data, client->player_data_size);
				send_message_to_player_in_room(client->player_id, buffer, packet_length);			

				client->player_data_set= TRUE;
			} else {
				if (synchronize_player_list(client, 0, client->player_id, _change_player_verb) == FALSE)
				{
					return TRUE;
				}
			}
		}
		else
		{
			return TRUE;
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in (tried a set_player_data)");
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_player_info_query_packet (
	struct client_data * client, 
	struct packet_header * header)
{
	char buffer[SCRATCH_BUFFER_SIZE];			
	struct player_info_query_packet * packet;
	char * p;
	short packet_length;

	p = (char *)header;
	p += sizeof(struct packet_header);
	packet = (struct player_info_query_packet *)p;

	packet_length = build_rs_player_info_request_packet(buffer, client->player_id, packet->player_id);
	send_room_packet(room_globals.userd_socket, buffer, packet_length);

	return FALSE;
}

static boolean handle_game_search_query_packet(
	struct client_data * client, 
	struct packet_header * header)
{
	struct game_search_query_packet * packet;
	char * psz;
	char buffer[SCRATCH_BUFFER_SIZE];
	short s_packet_length;

	if (client->logged_in)
	{
		char * psz_game_name, * psz_map_name;

		psz = (char *)header;
		psz += sizeof(struct packet_header);
		packet = (struct game_search_query_packet *)psz;
		psz += sizeof(struct game_search_query_packet);
		psz_game_name = psz;
		psz += strlen(psz_game_name) + 1;
		psz_map_name = psz;

		s_packet_length = build_gs_query_packet(buffer, get_client_player_id(client), psz_game_name, psz_map_name,
			packet->s_game_scoring, packet->unit_trading, packet->veterans, packet->teams, packet->alliances, 
			packet->enemy_visibility);
		send_game_search_packet(room_globals.game_search_socket, buffer, s_packet_length);
	}

	return FALSE;
}


// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
static boolean handle_player_search_query_packet(
	struct client_data * client, 
	struct packet_header * header)
{	
	char * p;
	char buffer[SCRATCH_BUFFER_SIZE] = {0};
	short packet_length;
	unsigned long buddies[MAXIMUM_BUDDIES] = {0};

	if (client->logged_in)
	{
		p = (char *)header;
		p += sizeof(struct packet_header);

		buddies[0] = 0;
		packet_length = build_rs_player_query_packet(buffer, get_client_player_id(client), &buddies[0], 0, p);		
		send_room_packet(room_globals.userd_socket, buffer, packet_length);
	}

	return FALSE;
}

static boolean handle_order_query_packet(
	struct client_data * client, 
	struct packet_header * header)
{
	short packet_length;
	unsigned long buddies[MAXIMUM_BUDDIES] = {0};
	char buffer[SCRATCH_BUFFER_SIZE] = {0};

	if (client->logged_in)
	{
		buddies[0] = 0;
		packet_length = build_rs_player_query_packet(buffer, get_client_player_id(client), &buddies[0], client->order, "");
		send_room_packet(room_globals.userd_socket, buffer, packet_length);
	}

	return FALSE;
}

static boolean handle_buddy_query_packet(
	struct client_data * client, 
	struct packet_header * header)
{
	unsigned long buddies[MAXIMUM_BUDDIES] = {0};
	char buffer[SCRATCH_BUFFER_SIZE] = {0};
	short packet_length;
	short n;

	if (client->logged_in)
	{
		for (n = 0; n < MAXIMUM_BUDDIES; ++n)
		{
			buddies[n] = client->buddies[n].player_id;
		}
		packet_length = build_rs_player_query_packet(buffer, get_client_player_id(client), buddies, 0, "");
		send_room_packet(room_globals.userd_socket, buffer, packet_length);
	}

	return FALSE;
}

static boolean handle_update_player_information_packet(
	struct client_data * client,
	struct packet_header * header)
{
	short packet_length;
	char * p;

	char buffer[SCRATCH_BUFFER_SIZE];			

	p = (char *)header;
	p += sizeof(struct packet_header);

	packet_length = build_rs_update_player_information_packet(buffer, client->player_id, p);
	send_room_packet(room_globals.userd_socket, buffer, packet_length);

	return FALSE;
}
#endif
// ALAN End

static boolean handle_directed_data_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		unsigned long player_id;
		boolean *local_echo;
		char *actual_data= (char *) header+sizeof(struct packet_header)+sizeof(unsigned long)+sizeof(boolean);
		short actual_length= header->length-sizeof(struct packet_header)+sizeof(unsigned long)+sizeof(boolean);
		
		player_id= *((long *)((byte *) header+sizeof(struct packet_header)));
		local_echo= ((boolean *) ((byte *) header+sizeof(struct packet_header)+sizeof(unsigned long)));

		if(!handle_remote_command(client, actual_data, actual_length))
		{
			if(client->flags & _client_is_gagged_flag)
			{
				if (send_room_message_to_player(client, "You have been gagged. If you feel this was unfair, send email to your@email.com") == FALSE)
				{
					return TRUE;
				}
			}
			else if (header->length != 0)
			{
				if (send_message_to_player_in_room(player_id, (char *) header, header->length) == FALSE)
				{
					return TRUE;
				}

				if((*local_echo) && player_id!=client->player_id)
				{
					*local_echo= FALSE;
					if (send_message_to_player_in_room(client->player_id, (char *) header, header->length) == FALSE)
					{
						return TRUE;
					}
				}
			}
		}
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in! (Tried a @pemit)");
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_set_player_mode_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;
	short response_code= NONE;

	if(client->logged_in)
	{
		short new_player_mode;
		char buffer[SCRATCH_BUFFER_SIZE];
			
		new_player_mode= *((short *)((byte *) header+sizeof(struct packet_header)));
		if (new_player_mode>=0 && new_player_mode<NUMBER_OF_PLAYER_MODES)
		{
			client->player_mode= new_player_mode;
			client->deaf= client->player_mode==_player_mode_deaf;
			sprintf(buffer, "set their deafness to %s", 
				client->player_mode==_player_mode_deaf ? "TRUE" : "FALSE");
			log_user_message(client, buffer);
		}
		else
		{
			return TRUE;	
		}		
	} else {
		response_code= _user_not_logged_in_msg;
		log_user_message(client, "not logged in (tried to set their deafness)");
	}

	done |= send_response(client, response_code);

	return done;
}

static boolean handle_request_full_update_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;

	UNUSED_PARAMETER(header);

	if(client->logged_in)
	{
		if(!client->deaf)
		{
			if (send_full_update_to_client(client) == FALSE)
			{
				done = TRUE;
			}
		}
	} else {
		log_user_message(client, "not logged in (requested full update)");
	}

	return done;
}

static boolean handle_game_player_list_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;

	if(client->logged_in)
	{
		struct player_list_packet_entry *base= (struct player_list_packet_entry *) ((char *) header+sizeof(struct packet_header)+sizeof(long));
		long game_id= *(long *) ((char *) header+sizeof(struct packet_header));
		short player_count= ((header->length-sizeof(struct packet_header)-sizeof(long))/sizeof(struct player_list_packet_entry));

		if(game_id && client->game_id)
		{
			game_id= client->game_id;
		}
		
		if(header->length==sizeof(struct packet_header)+sizeof(long)+player_count*sizeof(struct player_list_packet_entry))
		{
			short index;

			reset_player_list(game_id);

			for(index= 0; index<player_count; ++index)
			{
				struct player_list_packet_entry *entry= base+index;
				struct player_stats stats;
				
				if(get_logged_in_player_id_stats(entry->player_id, &stats))
				{
					add_player_to_game(game_id, entry->player_id);
				}
			}
		} else {
			log_user_message(client, "sent an incomplete game player list packet");
		}
	} else {
		log_user_message(client, "not logged in (sent a game player list packet)");
	}

	return done;
}

static boolean handle_game_score_packet(
	struct client_data * client,
	struct packet_header * header)
{
	struct game_score_packet * packet;
	unsigned long game_id;
	char * p;

	p = (char *)header;
	p += sizeof(struct packet_header);

	packet = (struct game_score_packet *)p;

	if (client->logged_in)
	{
		game_id = client->game_id ? client->game_id : packet->game_id;
		report_standings_for_game(game_id, client->player_id, &packet->standings);
	}

	return FALSE;
}

static boolean handle_start_game_packet(
	struct client_data *client,
	struct packet_header *header)
{
	boolean done= FALSE;

	if(client->logged_in)
	{
		if(header->length==sizeof(struct packet_header)+sizeof(struct start_game_packet))
		{
			struct start_game_packet *packet= (struct start_game_packet *) ((char *) header+sizeof(struct packet_header));
		
			if(client->game_id)
			{
				struct game_data *game= find_game_by_id(client->game_id);
				// ALAN Begin: unused variable
			//	char buffer[SCRATCH_BUFFER_SIZE];
				// ALAN End

				client->game_start_ticks= machine_tick_count();

				if (game)
				{
					game->reported= FALSE;
				}

				if(packet->game_time_in_seconds>0)
				{
					client->game_length_in_seconds= packet->game_time_in_seconds;

					output_games_log_message("game started", room_globals.room_identifier, client->game_id);
				} else {
					client->game_length_in_seconds= NONE;

				}
			} else {
				log_user_message(client, "tried to start a game, and didn't have one!");
			}
		} else {
			log_user_message(client, "sent an incomplete start game packet!");
		}
	} else {
		log_user_message(client, "not logged in (sent a start game packet)");
	}

	return done;
}

/* ALAN Begin: this function defined but not used
static boolean send_player_to_url(
	struct client_data *client, 
	short url_type,
	char *message,
	char *url)
{
	short response_length;
	char buffer[1024];

	response_length= build_url_packet(buffer, url_type, message, url);
	if(response_length>=sizeof(buffer))
		return FALSE;
	return add_packet_to_outgoing_buffer(client, buffer, response_length);
}
// ALAN End  */

static boolean send_response(
	struct client_data *client,
	short response_code)
{
	boolean done= FALSE;
	
	if(response_code != NONE)
	{
		short response_length;
		char buffer[1024];
	
		response_length= build_server_message_packet(buffer, response_code);
		if(response_length<=sizeof(buffer))
			done= add_packet_to_outgoing_buffer(client, buffer, response_length);
	}

	return done;
}

static void log_user_message(
	struct client_data *client,
	char *message)
{
	return;
}

static boolean add_game_to_room(
	struct client_data *client,
	short game_port,
	short order,
	char *data,
	short game_data_length)
{
	short game_classification= 0;

	if (!order && !room_globals.ranked_room && 
		!room_globals.tournament_room)
	{
		game_classification = _unranked_normal;
	}
	else if (order && !room_globals.ranked_room &&
		!room_globals.tournament_room)
	{
		game_classification = _unranked_order;
	}
	else if (!order && room_globals.ranked_room &&
		!room_globals.tournament_room)
	{
		game_classification = _ranked_normal;
	}
	else if (order && room_globals.ranked_room &&
		!room_globals.tournament_room)
	{
		game_classification = _ranked_order;
	}
	else if (!order && room_globals.tournament_room)
	{
		game_classification = _tournament_normal;
	}
	else if (order && room_globals.tournament_room)
	{
		game_classification = _tournament_order;
	}
	else
	{
		vhalt("unknown game classification?");
	}

	// copy it across.
	if(game_data_length<0 || game_data_length>MAXIMUM_METASERVER_GAME_DATA_SIZE)
		return FALSE;
	memcpy(client->game_data, data, game_data_length);
	client->game_data_size= game_data_length;
	client->game_id= create_game(client->player_id, game_classification, game_data_length, data);
	client->game_port= game_port;

	if (synchronize_game_list(client, 0, client->game_id, _add_game_verb) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

static boolean change_game_in_room(
	struct client_data *client,
	short order,
	char *data,
	short game_data_length)
{
	short game_classification= 0;

	if (!order && !room_globals.ranked_room && 
		!room_globals.tournament_room)
	{
		game_classification = _unranked_normal;
	}
	else if (order && !room_globals.ranked_room &&
		!room_globals.tournament_room)
	{
		game_classification = _unranked_order;
	}
	else if (!order && room_globals.ranked_room &&
		!room_globals.tournament_room)
	{
		game_classification = _ranked_normal;
	}
	else if (order && room_globals.ranked_room &&
		!room_globals.tournament_room)
	{
		game_classification = _ranked_order;
	}
	else if (!order && room_globals.tournament_room)
	{
		game_classification = _tournament_normal;
	}
	else if (order && room_globals.tournament_room)
	{
		game_classification = _tournament_order;
	}
	else
	{
		vhalt("unknown game classification?");
	}

	if(game_data_length<0 || game_data_length>MAXIMUM_METASERVER_GAME_DATA_SIZE)
		return FALSE;
	memcpy(client->game_data, data, game_data_length);
	client->game_data_size= game_data_length;
	change_game_information(client->game_id, game_classification, game_data_length, data);

	if (synchronize_game_list(client, 0, client->game_id, _change_game_verb) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

static boolean delete_game_from_room(
	struct client_data *client)
{
	delete_registered_game(client);
	delete_game(client->game_id);
	if (synchronize_game_list(client, 0, client->game_id, _delete_game_verb) == FALSE)
	{
		return FALSE;
	}
	client->game_id= 0;
	
	return TRUE;
}

// call with NONE, NONE to send a full update to everyone in the room.
static boolean synchronize_player_list(
	struct client_data *client,
	unsigned long player_to_send_to,
	unsigned long player_id_changed,
	short verb)
{
	short length, header_only_length;
	struct client_data *player= first_client;
	char buffer[MEDIUM_BUFFER_SIZE] = {0};

	length= start_building_player_list_packet(buffer);
	header_only_length= length;
	while(player)
	{
		if(!player_id_changed || player_id_changed==player->player_id)
		{
			// because you could get a player list packet before you are logged in..
			if(player->player_data_size)
			{
				struct metaserver_player_aux_data player_aux_data;

				memset(&player_aux_data, 0, sizeof(struct metaserver_player_aux_data));
				player_aux_data.verb= verb;
				player_aux_data.flags= player->metaserver_aux_data_flags;
				player_aux_data.ranking= player->caste;
				player_aux_data.player_id= player->player_id;
				player_aux_data.caste= (player->player_is_guest || !room_globals.ranked_room) ? NONE : player->caste;
				player_aux_data.player_data_length= player->player_data_size;
				player_aux_data.order= player->order;
			
				if((length+player->player_data_size) < sizeof(buffer))
				{
					length= add_player_data_to_packet(buffer, &player_aux_data, 
						player->player_data, player->player_data_size);
				}
			}
		}
		player= player->next;
	}
	
	if(header_only_length!=length)
	{
		if (send_message_to_player_in_room(player_to_send_to, buffer, length) == FALSE)
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

static boolean synchronize_game_list(
	struct client_data *client,
	unsigned long player_to_send_to,
	unsigned long game_id_changed,
	short verb)
{
	short length, header_only_length;
	struct client_data *player= first_client;
	char buffer[MEDIUM_BUFFER_SIZE] = {0};

	length= start_building_game_list_packet(buffer);
	header_only_length= length;
	while(player)
	{
		if(!game_id_changed || player->game_id==game_id_changed)
		{
			if(player->game_id)
			{
				struct metaserver_game_aux_data aux_data;

				memset(&aux_data, 0, sizeof(struct metaserver_game_aux_data));
				aux_data.game_id= player->game_id;
				aux_data.host= player->host;
				aux_data.port= player->game_port;
				aux_data.verb= verb;
				aux_data.version= player->version;
				if(player->game_length_in_seconds == NONE) // infinite games
				{
					aux_data.seconds_remaining= NONE;
				} else {
					aux_data.seconds_remaining= player->game_length_in_seconds-((machine_tick_count()-player->game_start_ticks)/MACHINE_TICKS_PER_SECOND);
				}
				aux_data.creating_player_id= player->player_id;
				aux_data.game_data_size= player->game_data_size;
				length= add_game_data_to_packet(buffer, &aux_data, player->game_data, player->game_data_size);
			}
		}
		player= player->next;
	}
	if(length>sizeof(buffer))
		return FALSE;
	
	if(header_only_length != length)
	{
		if (send_message_to_player_in_room(player_to_send_to, buffer, length) == FALSE)
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

static boolean remove_client_from_room(
	struct client_data *client)
{
	client->deaf= TRUE;

	if(client->game_id)
	{
		delete_game_from_room(client);
	}

	if (synchronize_player_list(client, 0, client->player_id, _delete_player_verb) == FALSE)
	{
		return FALSE;
	}
	client->logged_in= FALSE;
	
	return TRUE;
}

static boolean send_full_update_to_client(
	struct client_data *client)
{
	if (synchronize_player_list(client, client->player_id, 0, _add_player_verb) == FALSE)
	{
		return FALSE;
	}
	if (synchronize_game_list(client, client->player_id, 0, _add_game_verb) == FALSE)
	{
		return FALSE;
	}
	
	return TRUE;
}

static boolean send_room_motd(
	struct client_data *client)
{
	boolean done= FALSE;
	
	if(strlen(room_globals.motd))
	{
		done= send_room_message_to_player(client, room_globals.motd);
	}
	
	return done;
}

static boolean player_in_room(
	unsigned long player_id,
	struct client_data *ignore)
{
	struct client_data *client= first_client;
	boolean in_room= FALSE;
	
	while(client)
	{
		if(client != ignore && client->player_id==player_id)
		{
			in_room= TRUE;
			break;
		}
		client= client->next;
	}
	
	return in_room;
}

static unsigned long find_guest_user_id(
	void)
{
	unsigned long guest_id= guest_user_base_id;
	boolean done= FALSE;
	
	while(!done)
	{
		struct client_data *client= first_client;
		
		while(client)
		{
			if(client->player_id==guest_id)
			{
				// next!
				guest_id+= 1;
				break;
			}
			client= client->next;
		}
		
		if(!client) done= TRUE;
	}
	
	return guest_id;
}

/* _MYTHDEV Begin:  Original function (for reference)
static boolean add_packet_to_outgoing_buffer(
	struct client_data *client,
	char *buffer,
	short length)
{
	boolean success = TRUE;

	if (byteswap_packet(buffer, TRUE) == TRUE)
	{
		success= copy_data_into_circular_queue(buffer, length, &client->outgoing_buffer);
		if(!success)
		{
		}
		if (byteswap_packet(buffer, FALSE) != TRUE)	
		{
			success = FALSE;
		}
	}

	return (!success);
}
// _MYTHDEV End  */

static boolean add_packet_to_outgoing_buffer(
	struct client_data *client,
	char *buffer,
	short length)
{
	boolean success = TRUE;

    // _MYTHDEV Begin
    //This method of backing up the buffer is arguable faster than
    //encrypting and byte swapping and then decrypting and byte swapping again.
    char* backup_buffer = malloc( length );
    int original_length = length;
    memcpy( backup_buffer, buffer, length ); 

	if ( client->valid_session_key ) {
         length = encryptData( client, buffer, length );
    } else {                                                                       
		byteswap_packet(buffer, TRUE);
    }     
    // _MYTHDEV End

    success= copy_data_into_circular_queue(buffer, length, &client->outgoing_buffer);

    // _MYTHDEV Begin:  Undo changes and leave buffer intact
    memcpy( buffer, backup_buffer, original_length ); 
    free ( backup_buffer );
    // _MYTHDEV End

	return (!success);
}


static boolean client_has_screen_name(
	struct client_data *client, 
	char *name)
{
	boolean matched= FALSE;

	switch(room_globals.game_type)
	{
		case 0:
			matched= myth_player_data_matchs_screen_name(client->player_data, client->player_data_size, name);
			break;
			
		default:
			break;
	}
	
	return matched;
}

struct myth_metaserver_player_data {
	char coat_of_arms_bitmap_index;
	char caste_bitmap_index;
	short state;
	struct rgb_color primary_color;
	struct rgb_color secondary_color;
};

static boolean myth_player_data_matchs_screen_name(
	char *player_data, 
	short player_data_size, 
	char *name)
{
	char screen_name[64];
	boolean matched= FALSE;

	strlwr(name);
	strcpy(screen_name, get_myth_screen_name_from_playerdata(player_data, player_data_size));
	strlwr(screen_name);

	if(strcmp(screen_name, name)==0)
	{
		matched= TRUE;
	}
	
	return matched;
}

static char *get_myth_screen_name_from_playerdata(
	char *player_data, 
	short player_data_size)
{
	UNUSED_PARAMETER(player_data_size);

	return player_data + sizeof(struct myth_metaserver_player_data);
}

static boolean is_room_ranked(
	void)
{
	return (room_globals.ranked_room);
}

static boolean add_new_game(
	struct client_data * p_client, 
	char * p_data, 
	int n_data_length)
{
	short s_packet_length;
	char packet_buffer[SCRATCH_BUFFER_SIZE];
	struct metaserver_game_aux_data aux_data;

	char * p;

	aux_data.game_id = p_client->game_id;
	aux_data.host = p_client->host;
	aux_data.port = p_client->game_port;
	aux_data.verb = 0;
	aux_data.version = 0;
	aux_data.seconds_remaining = p_client->game_length_in_seconds-((machine_tick_count()-p_client->game_start_ticks)/MACHINE_TICKS_PER_SECOND);
	aux_data.creating_player_id = p_client->player_id;
	aux_data.game_data_size = p_client->game_data_size;

	byte_swap_game_data(p_data);

	s_packet_length = build_gs_update_packet(packet_buffer, _add_new_game, 
		room_globals.room_identifier, is_room_ranked(), &aux_data, n_data_length, 
		(struct metaserver_game_description *)p_data);

	p = packet_buffer;
	p += sizeof(struct room_packet_header);

	return send_game_search_packet(room_globals.game_search_socket, packet_buffer, s_packet_length);
}

static boolean delete_registered_game(
	struct client_data * p_client)
{
	short s_packet_length;
	char packet_buffer[SCRATCH_BUFFER_SIZE];

	s_packet_length = build_gs_update_delete_packet(packet_buffer, _remove_game, p_client->game_id, room_globals.room_identifier, p_client->player_id);
	return send_game_search_packet(room_globals.game_search_socket, packet_buffer, s_packet_length);
}

static boolean change_registered_game(
	struct client_data * p_client, 
	char * p_data, 
	int n_data_length)
{
	short s_packet_length;
	char packet_buffer[SCRATCH_BUFFER_SIZE];

	struct metaserver_game_aux_data aux_data;

	aux_data.game_id = p_client->game_id;
	aux_data.host = p_client->host;
	aux_data.port = p_client->game_port;
	aux_data.verb = 0;
	aux_data.version = 0;
	aux_data.seconds_remaining = p_client->game_length_in_seconds-((machine_tick_count()-p_client->game_start_ticks)/MACHINE_TICKS_PER_SECOND);
	aux_data.creating_player_id = p_client->player_id;
	aux_data.game_data_size = p_client->game_data_size;

	byte_swap_game_data(p_data);

	s_packet_length = build_gs_update_packet(packet_buffer, _change_game_info, room_globals.room_identifier, is_room_ranked(), &aux_data, n_data_length, (struct metaserver_game_description *)p_data);

	return send_game_search_packet(room_globals.game_search_socket, packet_buffer, s_packet_length);

}

static boolean update_userd_on_player(
	unsigned long player_id,
	boolean log_in)
{
	char buffer[40];
	short length;

	if (log_in)
	{
		length = build_rs_player_enter_room_packet(buffer, player_id, room_globals.room_identifier);
	}
	else
	{
		length = build_rs_player_leave_room_packet(buffer, player_id, room_globals.room_identifier);
	}

	return send_room_packet(room_globals.userd_socket, buffer, length);
}

// _MYTHDEV Begin
static boolean decryptData( struct packet_header* packet_header, struct client_data * client_data ) 
{
	if ( client_data->valid_session_key  ) {
        char* pData = ((char*)packet_header + sizeof(struct packet_header));  
        #ifdef little_endian
            int data_size = SWAP4(packet_header->length) - sizeof(struct packet_header);
        #else
            int data_size = packet_header->length - sizeof(struct packet_header);
        #endif

        data_size = decodeData( pData, data_size, client_data->session_key );
        packet_header->length = data_size + sizeof(struct packet_header);
        #ifdef little_endian
            packet_header->length = SWAP4( packet_header->length );
        #endif
    }
    return packet_header->length >= sizeof(struct packet_header);
}
// _MYTHDEV End


// _MYTHDEV Begin
static int encryptData( struct client_data *client, char *buffer, short length)
{
    struct client_data* client_data = (struct client_data*)client;
    int header_size = sizeof(struct packet_header);     
    struct packet_header* packet_header = (struct packet_header*)buffer;
    char*  pData = buffer + header_size;
    int data_size = length - header_size;

    byteswap_packet(buffer, TRUE);

    data_size = encodeData( pData, data_size, client_data->session_key ); 

    packet_header->length = data_size + header_size;
    length = packet_header->length;    

    #ifdef little_endian
	    packet_header->length= SWAP4(packet_header->length);
    #endif
    
    return length;
}
// _MYTHDEV End

