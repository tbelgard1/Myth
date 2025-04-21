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
#include "../common/logging.h"
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include "environment.h"
#include "metaserver_common_structs.h"
#include "stats.h"
#include "authentication.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "room_list_file.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "metaserver_codes.h"
#include "room_packets.h"
#include "server_code.h"
#include "room_globals.h"
#include "network_queues.h"
#include "game_search_packets.h"
#include "games_log.h"
#include "../sqlite_utils.h"
#include "room_sql.h"
#include <sqlite3.h>

static sqlite3 *db = NULL;

enum {
	ROOM_PLAYER_LIMIT_SLOP= 15,
	DEFAULT_MAXIMUM_PLAYERS_PER_ROOM= 40,
	DEFAULT_SECONDS_BEFORE_UPDATE= 30,

	MAXIMUM_OUTSTANDING_REQUESTS= 32,
	SECONDS_TO_WAIT_ON_SELECT= 10,
	STDIN_SOCKET= 0,
	MAXIMUM_PACKET_SIZE= 512,
	INCOMING_BUFFER_SIZE= 1*MEG,
	SCRATCH_BUFFER_SIZE= (16*KILO),

	MAXIMUM_NUMBER_OF_ROOMS= 64,
	FIRST_ROOM_LISTENING_PORT= 6335
};

enum {
	_awaiting_login_user_state,
	_logged_in_state,
	NUMBER_OF_USER_STATES
};

/* ------- parmaeters structure */
struct user_parameters {
	long host;
	short port;
	short game_search_port;
	char password[16];
};

/* --------- globals */
struct global_room_data room_globals;
static struct user_parameters parameters;


static boolean b_ready_for_search = FALSE;

/* --------- local prototypes */
static void parse_command_arguments(int argc, char *argv[]);
static void run_server(void);
static boolean valid_remote_host(unsigned long host);
static boolean handle_userd_data(struct circular_queue *incoming);
static int connect_to_user_daemon(short room_port, short room_identifier);
static int connect_to_game_search_engine(long l_roomd_id);
static boolean handle_userd_packet(struct room_packet_header *header);
static void check_for_dropped_players(fd_set *readfds);
static int idle_room(fd_set *readfds, int set_size);
static boolean send_room_status(void);
static boolean update_player_room_list(struct room_packet_header *header);
static boolean handle_game_search_data(struct circular_queue *incoming);
static boolean handle_game_search_packet(struct room_packet_header *header);
static boolean handle_gs_query_response_packet(struct room_packet_header * header);
static boolean handle_rs_player_query_response_packet(
	struct room_packet_header * header);
static boolean handle_rs_player_info_reply_packet(
	struct room_packet_header * header);

extern struct client_data * find_client_from_player_id(unsigned long ul_player_id);

/* --------- code */

// ALAN Begin: who does this?
//void main(
int main(
// ALAN End
	int argc,
	char *argv[])
{
    printf("[INFO] roomd_new starting up...\n");
    fflush(stdout);
    // Initialize SQLite DB and tables
    if (!initialize_all_tables(&db)) {
        fprintf(stderr, "Failed to initialize SQL database.\n");
        return 1;
    }

	memset(&room_globals, 0, sizeof(struct global_room_data));
	room_globals.userd_socket= NONE;
	room_globals.game_search_socket = NONE;
	room_globals.room_port= NONE;
	room_globals.room_identifier= NONE;
	room_globals.seconds_before_update= DEFAULT_SECONDS_BEFORE_UPDATE;
	room_globals.maximum_players_per_room= DEFAULT_MAXIMUM_PLAYERS_PER_ROOM;
	
	room_globals.launch_time = time(NULL);

	initialize_games_log();

	parse_command_arguments(argc, argv);

    printf("[INFO] roomd_new entering main server loop...\n");
    fflush(stdout);
    run_server();
    printf("[ERROR] roomd_new main server loop exited unexpectedly!\n");
    fflush(stdout);
    printf("[INFO] roomd_new shutting down.\n");
    fflush(stdout);
    return 0;
}

void change_room_motd(
	char *motd)
{
	strcpy(room_globals.motd, motd);
	
	return;
}

/* --------- local code */
static void parse_command_arguments(
	int argc, 
	char *argv[])
{
	char host_name[256];
	int opt;
	struct hostent *hostdata;
	
	strcpy(host_name, get_userd_host());
    // [DEBUG] CASCADE HARDCODE TEST - roomd_new.c
    printf("[DEBUG] CASCADE HARDCODE TEST - roomd_port about to be set\n");
    parameters.port = 6322; // roomd: 6322
    printf("[INFO] roomd_port forcibly set to 6322 to match client.\n");
	parameters.game_search_port = DEFAULT_GAME_SEARCH_PORT;
	strcpy(parameters.password, "login");

	// parse command line options..
	while((opt= getopt(argc, argv, "dh:r:p:m:")) != -1)
	{
		switch(opt)
		{
			case 'r':
				strcpy(parameters.password, optarg);
				break;
				
			case 'p':
				parameters.port= atoi(optarg);
				break;
				
			case 'h':
				strcpy(host_name, optarg);
				break;

			case 'm':
				room_globals.maximum_players_per_room= atoi(optarg);
				break;
				
			case '?':
				exit(0);
				break;
				
			case ':':
				break;
		}
	}

	hostdata= gethostbyname(host_name);
	if(hostdata)
	{
		if(hostdata->h_addrtype != AF_INET)
		{
			exit(1);
		} else {
			struct in_addr *address;
			
			address= (struct in_addr *) *hostdata->h_addr_list;
			
			parameters.host= ntohl(address->s_addr);
		}
	} else {
		exit(1);
	}

	return;
}

fd_set readfds;
static void run_server(
	void)
{
	int server_socket;

    log_debug("roomd_new: entered main server loop");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_error("socket() failed: %s", strerror(errno));
        exit(1);
    }
    log_debug("socket() succeeded, fd=%d", sockfd);
    server_socket = sockfd;
    if(server_socket!=NONE) 
    {
		int set_size= 0, i;
		struct sockaddr_in server_address;
		// ALAN Begin: unused variable
	//	int reuseaddress= TRUE;
		// ALAN End
		boolean bound= FALSE;

		server_address.sin_family= AF_INET;
		server_address.sin_addr.s_addr= htonl(INADDR_ANY);
		server_address.sin_port= 0;

		signal(SIGPIPE, SIG_IGN);

		for (i= 0; !bound && (i < MAXIMUM_NUMBER_OF_ROOMS); i++)
		{
			server_address.sin_port= htons(FIRST_ROOM_LISTENING_PORT+i);
			if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address))==0)
			{
				bound= TRUE;
			}
		}

        if (bound)
        {
            log_debug("Calling listen() on server_socket...");
            if (listen(server_socket, SOMAXCONN) < 0) {
                log_error("listen() failed: %s", strerror(errno));
                return;
            }
            log_debug("listen() succeeded. Waiting for accept()...");

            printf("[DEBUG] roomd_new: Successfully bound server_socket to port %d\n", ntohs(server_address.sin_port));
            fflush(stdout);
            boolean done= FALSE;
            socklen_t length;
            
            length= sizeof(struct sockaddr_in);
            getsockname(server_socket, (struct sockaddr *) &server_address, &length);
            assert(length==sizeof(struct sockaddr_in));
            
            room_globals.room_port= ntohs(server_address.sin_port);
            room_globals.userd_socket= connect_to_user_daemon(room_globals.room_port, NONE);
            if(room_globals.userd_socket != NONE)
            {
                printf("[DEBUG] roomd_new: Connected to userd, entering main accept loop...\n");
                fflush(stdout);
                struct circular_queue userd_incoming;
                struct circular_queue game_search_incoming;
			
				if(!allocate_circular_queue("Userd incoming", &userd_incoming, INCOMING_BUFFER_SIZE))
				{
					exit(0);
				}

				if (!allocate_circular_queue("gs incoming", &game_search_incoming, INCOMING_BUFFER_SIZE))
				{
					exit(-1);
				}

                FD_ZERO(&readfds);
                FD_SET(server_socket, &readfds);
                FD_SET(room_globals.userd_socket, &readfds);
                set_size= MAX(server_socket+1, room_globals.userd_socket+1);

                time(&room_globals.last_userd_ping);

				while(!done) 
				{
					fd_set testfds_read;
					fd_set testfds_write;
					int result;
					struct timeval timeout;					

					testfds_read= readfds;
					build_client_writefds(&testfds_write);
					
					timeout.tv_sec= SECONDS_TO_WAIT_ON_SELECT;
					timeout.tv_usec= 0;
					result= select(set_size, &testfds_read, &testfds_write, (fd_set *) 0, &timeout);

					if((result<0) && (errno != EINTR))
					{
						exit(1);
					} 
					else if(result==0)
					{
						set_size= idle_room(&readfds, set_size);
					}
					else
					{
						int fd;
						int current_set_size= set_size;

						for(fd= 0; fd<current_set_size; fd++) 
						{
							if(FD_ISSET(fd, &testfds_read)) 
							{
								if(fd==server_socket) 
								{
									struct sockaddr_in remote_address;
									int client_socket, client_len;
									long host;
									short port;

									errno= 0;
									client_len= sizeof(struct sockaddr_in);
									client_socket= accept(server_socket, 
										(struct sockaddr *) &remote_address,
										&client_len);
									if(client_socket != NONE)
									{
										host= ntohl(remote_address.sin_addr.s_addr);
										port= ntohs(remote_address.sin_port);
										
										if(room_globals.logged_into_userd && valid_remote_host(host)) 
										{
											if (current_client_count()<room_globals.maximum_players_per_room+ROOM_PLAYER_LIMIT_SLOP)
											{

												if(add_client(client_socket, host, port))
												{	
													int flags= fcntl(client_socket, F_GETFL, 0);
													
													fcntl(client_socket, F_SETFL, O_NONBLOCK|flags);
													FD_SET(client_socket, &readfds);
													set_size= MAX(set_size, client_socket+1);
												} else {
													close(client_socket);
												}
											} else {
												refuse_client_room_full(client_socket);
												close(client_socket);
											}
										}
										else
										{
											close(client_socket);
										}
									}
									else
									{
									}
								} 
								else if(fd==room_globals.userd_socket)
								{
									int nread;
									boolean remove_userd= FALSE;

									ioctl(fd, FIONREAD, &nread);

									if(nread==0) 
									{
 										remove_userd= TRUE;
									}
									else
									{
										char buffer[INCOMING_BUFFER_SIZE];
			
										if(nread >= sizeof(buffer))
											nread = sizeof(buffer) - 1;
										nread = read(fd, buffer, nread);

										if(copy_data_into_circular_queue(buffer, nread, &userd_incoming) == FALSE)
										{
											remove_userd = TRUE;
										}
										else
										{
											remove_userd= handle_userd_data(&userd_incoming);
										}
									}
									
									if(remove_userd)
									{
										FD_CLR(fd, &readfds);
										close(fd);
										room_globals.userd_socket= NONE;
									}
								} 
								else if (fd == room_globals.game_search_socket)
								{
									int nread;
									boolean remove_game_search = FALSE;

									ioctl(fd, FIONREAD, &nread);
									if(nread==0) 
									{
 										remove_game_search = TRUE;
									}
									else
									{
										char buffer[64*KILO];
 			
										if(nread >= sizeof(buffer))
											nread = sizeof(buffer) -1;
										nread = read(fd, buffer, nread);

										if(copy_data_into_circular_queue(buffer, nread, &game_search_incoming) == FALSE)
										{
											remove_game_search = TRUE;
										}
										else
										{
											remove_game_search = handle_game_search_data(&game_search_incoming);
											if (remove_game_search)
											{
											}
										}
									}
									
									if(remove_game_search)
									{
										FD_CLR(fd, &readfds);
										close(fd);
										room_globals.game_search_socket= NONE;
									}
								}
								else 
								{
									int nread;
									struct client_data *client;

									client= find_client_from_socket(fd);
									if(client) 
									{
										boolean remove_client= FALSE;
										boolean client_has_outgoing_data= FALSE;
											
										ioctl(fd, FIONREAD, &nread);
										if(nread==0) 
										{
											remove_client= TRUE;
										}
										else
										{
											char buffer[SCRATCH_BUFFER_SIZE];

											if (nread >= sizeof(buffer))
											{
												remove_client = TRUE;
											}
											else
											{
												nread = read(fd, buffer, nread);

												if (copy_data_into_client_buffer(client, buffer, nread) == FALSE)
												{
													remove_client = TRUE;
												}
												else
												{					
													remove_client= handle_client_data(client, &client_has_outgoing_data);
												}
											}
										}			
										if(remove_client) 
										{
											closedown_client(client);
											delete_client(client);
											FD_CLR(fd, &readfds);
											FD_CLR(fd, &testfds_write);
											close(fd);
										}
										else
										{
											if(client_has_outgoing_data)
											{
												FD_SET(fd, &testfds_write);
											}
										}
									}
									else
									{
									}
								}
							}

							if(FD_ISSET(fd, &testfds_write))
							{
								struct client_data *client;

								client= find_client_from_socket(fd);
								if(client) 
								{
									boolean remove_client= FALSE;
		
									remove_client= send_outgoing_data(client);
									if(remove_client) 
									{
										closedown_client(client);
										delete_client(client);
										FD_CLR(fd, &readfds);
										close(fd);
									} 
								}
								else
								{
								}
							}
						}

						set_size= idle_room(&readfds, set_size);
					}
					
				}
				
				free_circular_queue(&userd_incoming);
			} else {
			}
		}
		close(server_socket);
	}


	return;
}

#define	TEN_MINUTES		600
#define	SD_BOTH			2

static int idle_room(
	fd_set *readfds,
	int set_size)
{
	static long ticks_at_last_connection_attempt= 0l;
	static long ticks_at_last_room_update= 0l;
	static boolean b_game_search_initial_connect = TRUE;
	time_t current_time;

	time(&current_time);

	check_for_dropped_players(readfds);
	idle_games();

	if (current_time-room_globals.last_userd_ping>TEN_MINUTES)
	{
		if (room_globals.userd_socket!=NONE) 
		{
			shutdown(room_globals.userd_socket, SD_BOTH);
			FD_CLR(room_globals.userd_socket, readfds);
			//pause();
		}

		room_globals.userd_socket= NONE;
		time(&room_globals.last_userd_ping);
	}

	if((room_globals.userd_socket==NONE || 
		room_globals.game_search_socket == NONE) && 
		((machine_tick_count() - ticks_at_last_connection_attempt) 
		> 60*MACHINE_TICKS_PER_SECOND))
	{
		if (room_globals.userd_socket == NONE)
		{
			room_globals.userd_socket= connect_to_user_daemon(room_globals.room_port, room_globals.room_identifier);
			if(room_globals.userd_socket==NONE)
			{
			} else {
				FD_SET(room_globals.userd_socket, readfds);
				set_size= MAX(set_size, room_globals.userd_socket+1);
			}
		}

		if (room_globals.game_search_socket == NONE && !b_game_search_initial_connect)
		{
			room_globals.game_search_socket = connect_to_game_search_engine(room_globals.room_identifier);
			if (room_globals.game_search_socket == NONE)
			{
			}
			else
			{
				FD_SET(room_globals.game_search_socket, readfds);
				set_size = MAX(set_size, room_globals.game_search_socket + 1);
			}
		}
		ticks_at_last_connection_attempt= machine_tick_count();	
	}
	
	if(room_globals.userd_socket!=NONE && machine_tick_count()-ticks_at_last_room_update>(room_globals.seconds_before_update*MACHINE_TICKS_PER_SECOND))
	{
		char buffer[512];
		short length;
	
		length= build_short_room_status_packet(buffer);
		send_room_packet(room_globals.userd_socket, buffer, length);
		ticks_at_last_room_update= machine_tick_count();
	}

	if (b_ready_for_search)
	{
		if  (room_globals.game_search_socket == NONE && b_game_search_initial_connect)
		{
			b_game_search_initial_connect = FALSE;
			room_globals.game_search_socket = connect_to_game_search_engine(room_globals.room_identifier);
			if (room_globals.game_search_socket == NONE)
			{
			}
			else
			{
				FD_SET(room_globals.game_search_socket, readfds);
				set_size = MAX(set_size, room_globals.game_search_socket + 1);
			}
		}
	}
	
	return set_size;
}

static boolean valid_remote_host(
	unsigned long host)
{
	return TRUE;
}

static int connect_to_game_search_engine(long l_room_id)
{
	int sock;
	boolean success= 0;
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock != NONE)
	{
		struct sockaddr_in local_address;
		
		local_address.sin_family = AF_INET;
		local_address.sin_addr.s_addr = htonl(INADDR_ANY);
		local_address.sin_port = 0;

		if (bind(sock, (struct sockaddr *)&local_address, sizeof(local_address)) != -1)
		{
			struct sockaddr_in server_address;

			// game search engine and user authentication must be on same machine
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = htonl(parameters.host);
			server_address.sin_port = htons(parameters.game_search_port);
			if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == 0)
			{
				short length;
				char buffer[512];

				length = build_gs_login_packet(buffer, l_room_id);
				success = send_game_search_packet(sock, buffer, length);
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

	if (!success)
	{
		if (sock != NONE)
		{
			close(sock);
			sock = NONE;
		}
	}

	return sock;
}

static int connect_to_user_daemon(
	short room_port,
	short room_identifier)
{
	int s;
	boolean success= FALSE;
	
	s= socket(PF_INET, SOCK_STREAM, 0);
	if(s!=NONE) 
	{
		struct sockaddr_in local_address;

		local_address.sin_family= AF_INET;
		local_address.sin_addr.s_addr= htonl(INADDR_ANY);
		local_address.sin_port= 0;

		if(bind(s, (struct sockaddr *) &local_address, 
			sizeof(local_address))!=-1) 
		{
			struct sockaddr_in server_address;

			server_address.sin_family= AF_INET;
			server_address.sin_addr.s_addr= htonl(parameters.host); /* we don't care where we go. */
			server_address.sin_port= htons(parameters.port); /* no port required. */

			if(connect(s, (struct sockaddr *) &server_address, sizeof(server_address))==0)
			{
				short length;
				char buffer[512];
			
				length= build_rs_login_packet(buffer, room_port, room_identifier, parameters.password);
				success= send_room_packet(s, buffer, length);
			}
		} else {
		}
	}
	
	if(!success)
	{
		if(s != NONE)
		{
			close(s);
			s= NONE;
		}
	}
	
	return s;
}

static boolean handle_userd_data(
	struct circular_queue *incoming)
{
	boolean disconnect_client= FALSE;
	char packet[64*KILO];
	struct room_packet_header *header;
	boolean byteswap= FALSE;
	boolean disconn_from_error = FALSE;

#ifdef little_endian
	byteswap= TRUE;
#endif

	header= (struct room_packet_header *) packet;
	while(!disconnect_client && get_next_room_packet_from_queue(incoming, header, sizeof(packet), byteswap, &disconn_from_error))
	{
		if (!byteswap_room_packet((char *) header, FALSE))
			disconnect_client= TRUE;
		else
			disconnect_client= handle_userd_packet(header);
	}

	if (disconn_from_error == TRUE)
	{
		disconnect_client = TRUE;
	}

	return disconnect_client;
}

static boolean handle_game_search_data(struct circular_queue *incoming)
{
	boolean disconnect_client = FALSE;
	char packet[64 * KILO]; 
	struct room_packet_header *header;
	boolean byteswap = FALSE;
	boolean disconn_from_error = FALSE;

#ifdef little_endian
	byteswap= TRUE;
#endif

	header = (struct room_packet_header *) packet;
	while(!disconnect_client && get_next_room_packet_from_queue(incoming, header, sizeof(packet), byteswap, &disconn_from_error))
	{
		byte_swap_game_search_packet((char *) header, FALSE);
		disconnect_client= handle_game_search_packet(header);
	}

	if (disconn_from_error == TRUE)
	{
		disconnect_client = TRUE;
	}

	return disconnect_client;
}

static boolean handle_userd_packet(
	struct room_packet_header *header)
{
	boolean disconnect= FALSE;
	
	switch(header->type)
	{
		case _rs_ping_packet:
			time(&room_globals.last_userd_ping);
			break;

		case _rs_rank_update_packet:
			{
				char * p;
				struct rs_rank_update_packet * packet;

				p = (char *)header;
				p += sizeof(struct room_packet_header);

				// ALAN Begin: assignment from incompatible pointer type
			//	packet = (struct rs_caste_breakpoint_packet *)p;
				packet = (struct rs_rank_update_packet *)p;
				// ALAN End
				memcpy(&room_globals.caste_breakpoints, &packet->caste_breakpoints, sizeof(struct caste_breakpoint_data));
				memcpy(&room_globals.overall_rank_data, &packet->overall_rank_data, sizeof(struct overall_ranking_data));
			}
			break;

		case _rs_public_announcement_packet:
			{
				char * message;

				message = (char *)header;
				message += sizeof(struct room_packet_header);
				send_room_message_to_everyone(message);
			}
			break;

		case _rs_global_message_packet:
			{
				struct rs_global_message_packet * packet;
				char * message;
				struct client_data * client;

				message = (char *)header;
				message += sizeof(struct room_packet_header);
				packet = (struct rs_global_message_packet *)message;
				message += sizeof(struct rs_global_message_packet);

				client = get_client_from_player_id(packet->player_id);
				send_room_message_to_player(client, message);
			}
			break;

		case _rs_login_successful_packet:
			{
				struct rs_login_successful_packet *packet= (struct rs_login_successful_packet *) ((byte *) header+sizeof(struct room_packet_header));
				char *url_for_update= extract_url_for_update_from_rs_login_successful_packet((char *) header);
				char *motd= extract_motd_from_rs_login_successful_packet((char *) header);

				room_globals.flags= 0;
				room_globals.logged_into_userd= TRUE;
				room_globals.game_type= packet->game_type;
				room_globals.room_identifier= packet->identifier;
				room_globals.ranked_room= packet->ranked;
				room_globals.tournament_room = packet->tournament_room;
				strcpy(room_globals.url_for_version_update, url_for_update);
				strcpy(room_globals.motd, motd);
				memcpy(&room_globals.caste_breakpoints, &packet->caste_breakpoints, sizeof(struct caste_breakpoint_data));
				b_ready_for_search = TRUE;
			}
			break;
			
		case _rs_login_failure_packet:
			disconnect= TRUE;
			break;

#ifdef BN2_FULLVERSION
		case _rs_player_information_packet:
			{
				struct rs_player_information_packet * packet = (struct rs_player_information_packet *) ((byte *) header+sizeof(struct room_packet_header));
				char * p = (char *)header;
				p += sizeof(struct room_packet_header) + sizeof(struct rs_player_information_packet);

				set_client_player_information(packet->player_id, packet->buddies, packet->order, packet->country_code, packet->player_is_admin, packet->player_is_bungie_employee, packet->account_is_kiosk, p);
			}
			break;
#endif
			
		case _rs_client_ranking_packet:
			{
				struct rs_client_ranking_packet *packet= (struct rs_client_ranking_packet *) ((byte *) header+sizeof(struct room_packet_header));

				if (set_client_ranking(packet->player_id, packet->flags, &packet->stats) == FALSE)
				{
					disconnect = TRUE;
				}
			}
			break;

		case _rs_motd_changed_packet:
			change_room_motd(extract_motd_from_rs_motd_changed_packet((char *) header));
			break;
			
		case _rs_send_status_packet:
			disconnect= !send_room_status();
			break;
			
		case _rs_room_list_packet:
			disconnect = update_player_room_list(header);
			break;
			
		case _rs_update_buddy_response_packet:
			{
				struct buddy_entry * buddies;
				struct rs_update_buddy_response_packet * packet;
				long player_id;
				char * p;

				p = (char *)header;
				p += sizeof(struct room_packet_header);

				packet = (struct rs_update_buddy_response_packet *)p;

				player_id = packet->player_id;
				buddies = packet->buddies;

				update_client_buddy_list(player_id, buddies);
				disconnect = FALSE;
			}
			break;

		case _rs_update_order_status_packet:
			{
				struct rs_update_order_status_packet * packet;
				struct order_member order_members[STEFANS_MAXIMUM_ORDER_MEMBERS];
				short index, num_members;
				char * p;

				p = (char *)header;
				p += sizeof(struct room_packet_header);

				packet = (struct rs_update_order_status_packet *)p;
				
				memset(order_members, 0, STEFANS_MAXIMUM_ORDER_MEMBERS * sizeof(struct order_member));
				num_members= MIN(STEFANS_MAXIMUM_ORDER_MEMBERS, packet->member_count);
				
				p += sizeof(struct rs_update_order_status_packet);
				for(index=0; index<num_members; index++)
				{
					order_members[index].player_id = ((struct order_member *)(p))->player_id;
					order_members[index].online = ((struct order_member *)(p))->online;
					p += sizeof(struct order_member);
				}

				send_order_update_to_player(packet->player_id, packet->member_count, &order_members[0]);
				disconnect = FALSE;
			}
			break;

		case _rs_player_query_response_packet:
			disconnect = handle_rs_player_query_response_packet(header);
			break;

		case _rs_player_info_reply_packet:
			disconnect = handle_rs_player_info_reply_packet(header);
			break;
			
		default:
			break;
	}
	
	return disconnect;
}

static boolean handle_rs_player_info_reply_packet(
	struct room_packet_header * header)
{
	struct rs_player_info_reply_packet * packet;
	char * p;

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_player_info_reply_packet *)p;

	send_player_info_packet_to_player(packet);
	return FALSE;
}

static boolean handle_rs_player_query_response_packet(
	struct room_packet_header * header)
{
	boolean disconnect = FALSE;
	struct client_data * client;
	struct player_query_response_header * qh;
	struct player_query_response_segment * qs;
	int index;
	int packet_length= 0;
	char buffer[SCRATCH_BUFFER_SIZE];
	char * p;

	p = (char *)header;
	p += sizeof(struct room_packet_header);
	qh = (struct player_query_response_header *)p;

	client = find_client_from_player_id(qh->player_id);
	if (client == NULL)
		return FALSE;

	switch (qh->type)
	{
	case _player_search_query:
		packet_length = start_building_player_search_list_packet(buffer);

		p += sizeof(struct player_query_response_header);
		qs = (struct player_query_response_segment *)p;

		for (index = 0; index < qh->number_of_responses; index++)
		{
			packet_length = add_player_data_to_search_packet(buffer, qs->aux_data.room_id, 
				&qs->aux_data, qs->player_data);
			p += sizeof(struct player_query_response_segment);
			qs = (struct player_query_response_segment *)p;
		}
		break;

	case _order_query:
		packet_length = start_building_order_list_packet(buffer);

		p += sizeof(struct player_query_response_header);
		qs = (struct player_query_response_segment *)p;

		for (index = 0; index < qh->number_of_responses; index++)
		{
			packet_length = add_player_data_to_order_list_packet(buffer, qs->aux_data.room_id, 
				&qs->aux_data, qs->player_data);
			p += sizeof(struct player_query_response_segment);
			qs = (struct player_query_response_segment *)p;
		}
		break;

	case _buddy_query:
		packet_length = start_building_buddy_list_packet(buffer);
		
		p += sizeof(struct player_query_response_header);
		qs = (struct player_query_response_segment *)p;

		for (index = 0; index < qh->number_of_responses; index++)
		{
			packet_length = add_player_data_to_buddy_list_packet(buffer, qs->room_id, 
				&qs->aux_data, qs->player_data);
			p += sizeof(struct player_query_response_segment);
			qs = (struct player_query_response_segment *)p;
		}
		break;
	}

	send_message_to_player_in_room(get_client_player_id(client), buffer, packet_length);

	return disconnect;	
}

static boolean handle_game_search_packet(struct room_packet_header *header)
{
	boolean disconnect= FALSE;
	
	switch(header->type)
	{
	case _gs_query_response_packet:
		disconnect = handle_gs_query_response_packet(header);
		break;
			
	default:
		break;
	}
	
	return disconnect;
}

static boolean handle_gs_query_response_packet(struct room_packet_header * header)
{
	struct client_data * p_client;
	struct gs_query_response_header * packet_header;
	struct gs_query_response_segment * p_seg;
	char * psz = (char *)header;
	int n_index, n_length;
	char buffer[65536];

	memset(buffer, 0, sizeof(buffer));

	psz += sizeof(struct room_packet_header);
	packet_header = (struct gs_query_response_header *)psz;
	psz += sizeof(struct gs_query_response_header);
	p_seg = (struct gs_query_response_segment *)psz;

	p_client = find_client_from_player_id(packet_header->player_id);
	if (!p_client)
	{
		return FALSE;
	}

	n_length = start_building_game_list_pref_packet(buffer);

	p_seg = (struct gs_query_response_segment *)psz;
	for (n_index = 0; n_index < packet_header->number_of_responses; n_index++)
	{
		byte_swap_game_data((char *)&p_seg->game);
		n_length = add_game_data_to_packet(buffer, &p_seg->aux_data, &p_seg->game, p_seg->game_data_length);
		psz += p_seg->segment_length;
		p_seg = (struct gs_query_response_segment *)psz;
	}

	send_message_to_player_in_room(get_client_player_id(p_client), buffer, n_length);

	return FALSE;
}

static boolean send_room_status(
	void)
{
	char buffer[2048];
	short length;
	
	length= build_room_status_packet(buffer);
	return send_room_packet(room_globals.userd_socket, buffer, length);
}

static boolean update_player_room_list(
	struct room_packet_header *header)
{
	struct player_room_list_data *rooms= (struct player_room_list_data *) ((char *) header+sizeof(struct room_packet_header));
	short room_count= (header->length-sizeof(struct room_packet_header))/sizeof(struct player_room_list_data);
		
	if(sizeof(struct room_packet_header)+(room_count*sizeof(struct player_room_list_data))!=header->length)
		return TRUE;
	
	if (send_room_list_to_clients(rooms, room_count) == FALSE)
	{
		return TRUE;
	}

	
	return FALSE;
}

static void check_for_dropped_players(
	fd_set *readfds)
{
	struct client_data *client= find_first_client();

	while(client)
	{
		struct client_data *next= find_next_client(client);
		int client_socket= get_client_socket(client);
	
		if(ping_client(client))
		{
			closedown_client(client);
			delete_client(client);
			FD_CLR(client_socket, readfds);
			close(client_socket);
		}
		client= next;
	}
	
	return;
}
