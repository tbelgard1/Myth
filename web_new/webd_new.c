
 /*
  * Copyright (c) 2002 Vishvananda Ishaya
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

// _MYTHDEV Begin

#include "cseries.h"
#include "get_userd_host.h"
#include "../sqlite_utils.h"
#include "web_session_sql.h"
#include <sqlite3.h>

static sqlite3 *db = NULL;

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

#include "web_server_packets.h"
#include "environment.h"
#include "room_packets.h"

struct
{
	int userd_socket;
	int last_packet_type;
}
web_globals;

enum {
	SECONDS_TO_WAIT_FOR_RESPONSE= 20,
	INCOMING_BUFFER_SIZE= (16*KILO),
	SCRATCH_BUFFER_SIZE= (16*KILO),
	OUTGOING_BUFFER_SIZE= (16*KILO),
	RESPONSE_TYPE_DIFFERENCE= 101
};


int connect_to_user_daemon(void);
void disconnect_from_user_daemon(int s);
short send_web_packet(int socket, char *buffer, short length);
short build_packet_from_args(char * buffer, int argc, char *argv[]);
void handle_response(void);

struct {
	char * short_command;
	char * long_command;
	short num_args;
	short (*builder)(char * buffer, char * args[]);
} commands[]= {
	{ "qm", "query_motd", 0, build_query_motd_packet},
	{ "cm", "change_motd", 1, build_change_motd_packet},
	{ "nu", "new_user", 2, build_new_user_packet},
	{ "cp", "change_password", 2, build_change_password_packet},
	{ "co", "create_order", 6, build_create_order_packet},
	{ "jo", "join_order", 4, build_join_order_packet},
	{ "lo", "leave_order", 2, build_leave_order_packet},
	{ "uo", "update_order", 6, build_update_order_information_packet},
	{ "qo", "query_order", 1, build_query_order_information_packet},
	{ "bp", "boot_player_from_order", 3, build_boot_player_from_order_packet},
	{ "la", "lock_account", 2, build_lock_account_packet},
	{ "ua", "unlock_account", 1, build_unlock_account_packet},
	{ "ra", "reset_account", 1, build_reset_account_packet},
	{ "rg", "reset_game_type", 2, build_reset_game_type_packet},
	{ "ro", "reset_order", 1, build_reset_order_packet},
};

#define NUMBER_OF_COMMANDS (sizeof(commands)/sizeof(commands[0]))

int main(int argc, char *argv[])
{
    // Initialize SQLite DB and tables
    if (!initialize_all_tables(&db)) {
        fprintf(stderr, "Failed to initialize SQL database.\n");
        return 1;
    }

	char buffer[OUTGOING_BUFFER_SIZE];
	int length;
	if(argc <= 1)
	{
		printf("error: no command specified\n");
		return 1;
	}
	web_globals.userd_socket = connect_to_user_daemon();				
	if(web_globals.userd_socket == NONE)
	{
		printf("error: can't connect to socket\n");
		return 1;
	}
	length = build_packet_from_args(buffer, argc, argv);
	if(length == 0)
	{
		printf("error: failed to build packet\n");
		return 1;
	}
	
	if(send_web_packet(web_globals.userd_socket, buffer, length) == FALSE)
	{
		printf("error: failed to send packet\n");
		return 1;
	}
	handle_response();

	disconnect_from_user_daemon(web_globals.userd_socket);
	return 0;
}
void handle_response(void)
{
	time_t start_time;

	start_time = time(NULL);
	while(TRUE)
	{
		int nread;
		char buffer[INCOMING_BUFFER_SIZE];
		struct room_packet_header * header;

		if(time(NULL) - start_time > SECONDS_TO_WAIT_FOR_RESPONSE)
		{
			printf("error: timed out while waiting for response from userd\n");
			return;
		}	
		ioctl(web_globals.userd_socket, FIONREAD, &nread);
		if(nread <= 0)
			continue;
		if(nread >= sizeof(buffer))
			nread = sizeof(buffer) - 1;
		printf("debug: attempting to receive a %d byte packet\n", nread);
		nread = read(web_globals.userd_socket, buffer, nread);
		header = (struct room_packet_header *)buffer;
		printf("debug: received a %d byte packet\n", nread);
		if(header->type == _web_server_response_packet)
		{
			long new_id;
			char * p = buffer;
			struct web_server_response_packet * packet;
			p += sizeof(struct room_packet_header);
			packet = (struct web_server_response_packet *)p;
			p += sizeof(struct web_server_response_packet);
			printf("debug: received a web server response packet\n");
			if(packet->response_type != web_globals.last_packet_type + RESPONSE_TYPE_DIFFERENCE)
			{
				printf("error: received the wrong type of response\n");
				return;
			}
			if(packet->operation_code != _success)
			{
				printf("failure\n");
				if(p != NULL && p[0] != '\0');
					printf("%s\n", p);
				return;
			}

			printf("success\n");
			switch(packet->response_type)
			{
				case _query_motd_response_packet:
					printf("motd: %s\n", p);
					break;
				case _new_user_response_packet:
					memcpy (&new_id, p, sizeof(new_id));
					printf("player_id: %ld\n", new_id);
					break;
				case _query_order_information_response_packet:
					printf("member_password: %s\n", p);
					p+= strlen(p) + 1;
					printf("url: %s\n", p);
					p+= strlen(p) + 1;
					printf("contact_email: %s\n", p);
					p+= strlen(p) + 1;
					printf("motto: %s\n", p);
					break;
				default:
					break;
			}
			return;
		}
		else
		{
			printf("error: received an unknown packet type\n");
			return;
		}
	}
}

short build_packet_from_args(char * buffer, int argc, char *argv[])
{
	int i;
	int remaining_args = argc - 2;
	for(i = 0; i < NUMBER_OF_COMMANDS; i++)
	{
		if(strcmp(commands[i].short_command, argv[1]) == 0 || strcmp(commands[i].long_command, argv[1]) == 0)
		{
			if(commands[i].num_args == remaining_args)
			{
				char ** rest;
				rest = argv;
				rest += 2;
				web_globals.last_packet_type = i;
				printf("debug: building a %s packet\n",commands[i].long_command);
				return commands[i].builder(buffer, rest);
			}
			else
			{
				printf("    // [DEBUG] CASCADE HARDCODE TEST - webd_new.c\n");
				printf("[DEBUG] CASCADE HARDCODE TEST - webd_port about to be set\n");
				printf("[INFO] webd_port forcibly set to 6323 to match client.\n"); // webd: 6323
				printf("error: command %s takes %d arguments\n", commands[i].long_command, commands[i].num_args);
				return 0;
			}
		} 
	}
	printf("error: %s is not a valid command\n", argv[1]);
	return 0;
}

short send_web_packet(
	int socket, 
	char *buffer, 
	short length)
{
	int last_sent;
	int left_to_send;
	char *p = buffer;
		
	left_to_send= length;
	while (left_to_send)
	{
		last_sent = send(socket, p, left_to_send, 0);
		if (last_sent==NONE)
		{
			return FALSE;
		}
		left_to_send-= last_sent;
		p+= last_sent;
	}
	return TRUE;
}

int connect_to_user_daemon()
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
			char host_name[256];
			struct hostent *hostdata;

			server_address.sin_family= AF_INET;
	
            strcpy(host_name, get_userd_host());
			hostdata= gethostbyname(host_name);
			if(hostdata)
			{
				if(hostdata->h_addrtype != AF_INET)
				{
					exit(1);
				} else {
					struct in_addr * hostaddress;
					
					hostaddress = (struct in_addr *) *hostdata->h_addr_list;
					server_address.sin_addr.s_addr= htonl(ntohl(hostaddress->s_addr));
				}
			} else {
				exit(1);
			}
			// [DEBUG] CASCADE HARDCODE TEST - webd_new.c
printf("[DEBUG] CASCADE HARDCODE TEST - webd_port about to be set\n");
server_address.sin_port = htons(6321); // userd: 6321
printf("[INFO] userd_port forcibly set to 6321 for userd connection.\n");

                int retries = 5;
                while (retries-- > 0) {
                    if (connect(s, (struct sockaddr *) &server_address, sizeof(server_address)) == 0) {
                        printf("debug: connected to server\n");
                        success = TRUE;
                        break;
                    }
                    perror("connect");
                    sleep(1);
                }
                if (!success) {
                    fprintf(stderr, "Failed to connect to userd after retries.\n");
                }
            }
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

void disconnect_from_user_daemon(int s)
{
	if(s != NONE)
	{
		close(s);
		s= NONE;
	}
}

// _MYTHDEV End
