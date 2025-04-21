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

/*
This code modified/cleaned up by Alan Wagner (alanw@cox.net) 08.16.03
to be readily built under linux gcc or Cygwin's Windows version of gcc

Important: LOCAL_AREA_NETWORK in this file, and information in "environment.h"
must be completed correctly to successfully operate this server.

In addition, the proper target platform and compiler flags must be set in 
"build_settings.txt" before compiling.
*/


#define SERVER

#include "cseries.h"
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h> // For timestamps in debug output
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "environment.h"
#include "metaserver_common_structs.h"
#include "stats.h"
#include "authentication.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "orders.h"
#include "room_list_file.h"
#include "byte_swapping.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "metaserver_codes.h"
#include "room_packets.h"
#include "web_server_packets.h"
#include "network_queues.h"

// ALAN Begin: Added Headers
#include <time.h>
// ALAN End

// _MYTHDEV Begin
#include "encodePackets.h"
// _MYTHDEV End

static boolean handle_change_motd(char * buffer);

enum {
	MAXIMUM_OUTSTANDING_REQUESTS= 16,
	MAXIMUM_BAD_LOGIN_ATTEMPTS= 1,
	NEW_USER_LOGIN_KEY_SIZE= 16,
	NEW_ORDER_KEY_SIZE= 12,
	PASSWORD_CHANGE_LOGIN_KEY_SIZE= 8,
	STDIN_SOCKET= 0,
	INCOMING_QUEUE_SIZE= 65536,
	OUTGOING_QUEUE_SIZE= 65536,	
	MAXIMUM_PACKET_LENGTH= 32767,
	TICKS_BEFORE_UPDATING_ROOM_LIST= 45*MACHINE_TICKS_PER_SECOND,
	SECONDS_TO_WAIT_ON_SELECT= 60,

	CLASS_C_NETMASK= 0xffffff00,

// _MYTHDEV Begin: Running locally should automatically set network
#ifdef RUNNING_LOCALLY
	LOCAL_AREA_NETWORK=0x7f000000, 
#else 
	LOCAL_AREA_NETWORK= 0xC0A80000, // 192.168.0.0
#endif
// _MYTHDEV End


	MINIMUM_NUMBER_OF_PLAYERS_IN_ORDER= 3,
	TIME_TO_EXPIRE_ORDER= 60*60*24*10,
	
	MINIMUM_PATCH_VERSION_REQUIRED= 3
};

struct user_parameters {
	short userd_port;
	short web_port;
	short room_port;
	short unused;
	struct room_data *rooms;
	char new_user_login[NEW_USER_LOGIN_KEY_SIZE];
	char room_login[ROOM_PASSWORD_SIZE];
	struct client_data *clients;
	char motd[1024];
	char stats_mail_address[128];
};

struct server_globals {
	int server_socket;
	int room_socket;
	int web_socket;
	int unused;
	int set_size;
	struct sockaddr_in server_address;
	fd_set fds_read;
	fd_set fds_write;
};

#define printf		my_printf
int my_printf(const char *format, ...);

enum {
	_player_client_type,
	_room_client_type,
	_web_client_type,
	_unused_client_type,
	NUMBER_OF_CLIENT_TYPES
};

#define CLIENT_DATA_HEADER \
	int socket; \
	long host; \
	short port; \
	short type; \
	char buffer[MAXIMUM_PACKET_LENGTH]; \
	short state; \
	void *next; \
	struct circular_queue incoming; \
	struct circular_queue outgoing;

struct client_data {
	CLIENT_DATA_HEADER
};
	
struct player_client_data {
	CLIENT_DATA_HEADER

	long user_id;
	short order;
	int game_type;
	char username[MAXIMUM_METASERVER_USERNAME+1];
	unsigned char salt[MAXIMUM_SALT_SIZE];
	short authentication_type;
	authentication_token token;
	short login_attempts;

	short platform_type;
	short country_code;
	
	boolean reset_player_data;
	char player_data[MAXIMUM_METASERVER_PLAYER_DATA_SIZE];
	short player_data_size;

// _MYTHDEV Begin
    char    session_key[SESSION_KEY_SIZE];
    boolean valid_session_key;
// _MYTHDEV End
};

struct room_client_data {
	CLIENT_DATA_HEADER
	
	boolean logged_in;
	short room_port;
	short game_type;
	short room_identifier;
	short player_count;
	short game_count;
	short country_code;
	short minimum_caste;
	short maximum_caste;
	short tournament_room;
	short ranked_room;
};

struct web_client_data {
	CLIENT_DATA_HEADER
};

enum {
	_awaiting_login_packet_state,
	_awaiting_password_response_packet_state,
	_awaiting_version_packet_state
};

static struct user_parameters user_parameters;
static struct server_globals userd_globals;

static void parse_command_arguments(int argc, char *argv[]);
static void run_server(void);
static boolean valid_remote_host(unsigned long host, short type);
static struct client_data *add_client(struct client_data *clients, int socket, long host, short port, short type);
static struct client_data *find_client_from_socket(struct client_data *clients, int socket);
static struct room_client_data * find_room_client_from_room_id(
	short room_id);
static struct client_data *delete_client(struct client_data *clients, struct client_data *dead);

// ALAN Begin: this function defined but not used
//static boolean send_packet(int socket, char *buffer, short length);
// ALAN End

static int create_listening_socket(short port);
static char * load_motd(void);
static void save_motd(void);
static void idle_userd(void);

extern void bungie_net_game_evaluate(
	long host_player_id,
	short game_classification,
	short player_count,
	struct bungie_net_player_datum **bungie_net_players,
	struct bungie_net_order_datum **bungie_net_orders, 
	struct bungie_net_game_standings **reported_standings);

// ALAN Begin: this fixes an implicit declaration of function compiler warning
// Note: you may want to make a "game_evaluator.h" file instead of this method
extern void scoring_datum_adjust_total(
	struct bungie_net_player_score_datum *score_by_game_types,
	struct bungie_net_player_score_datum *total_score);
// ALAN End

static void build_client_writefds(fd_set *write_fds);
static boolean add_room_packet_to_outgoing(struct room_client_data *client, char *buffer, short length);
static boolean add_packet_to_outgoing(struct client_data *client, char *buffer, short length);
static boolean send_outgoing_data(struct client_data *client);

/* -------- player client handling code */

// _MYTHDEV Begin
static boolean handle_session_key_packet(struct player_client_data *client, struct packet_header *header);
static boolean decryptData( struct packet_header* packet_header, struct player_client_data * player_client_data );
static int encryptData( struct client_data *client, char *buffer, short length);
// _MYTHDEV End

static boolean handle_client_data(struct player_client_data *client);
static boolean handle_login_packet(struct player_client_data *client, struct packet_header *header);
static boolean handle_password_response_packet(struct player_client_data *client, struct packet_header *header);
static void send_successful_login_sequence(struct player_client_data *client);
static short build_game_specific_login_packet(struct player_client_data *client);
static short build_myth_specific_login_packet(struct player_client_data *client);
static boolean handle_version_control_packet(struct player_client_data *client, struct packet_header *header);

/* --------- new user handling code */
static boolean handle_web_client_data(struct web_client_data *client);

/* --------- room handling code */
static boolean handle_room_data(struct room_client_data *client);
static void login_next_queued_room(struct client_data *clients);
static boolean try_to_login_client_room(struct room_client_data *client, short identifier_hint);
static void send_user_ranking_packet(struct room_client_data *client, long player_id);
static boolean handle_rs_player_leave_room_packet(
	struct room_client_data * client,
	struct room_packet_header * header);
static boolean handle_rs_player_enter_room_packet(
	struct room_client_data * client,
	struct room_packet_header * header);
static boolean handle_rs_player_query_packet(
	struct room_client_data * client,
	struct room_packet_header * header);
static boolean handle_rs_update_player_information_packet(
	struct room_client_data * client,
	struct room_packet_header * header);
static boolean handle_rs_player_info_request_packet(
	struct room_client_data * client, 
	struct room_packet_header * header);
static boolean handle_rs_ban_player_packet(
	struct room_client_data * client,
	struct room_packet_header * header);

// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
	static boolean handle_rs_score_game_packet(
		struct room_client_data * client,
		struct room_packet_header * header);
#endif
	// ALAN End

static void send_room_packet_to_all_connected_rooms(char *buffer, short length);
static boolean send_room_list_to_rooms(boolean force_send);
#ifdef BN2_FULLVERSION
static void update_rooms_on_rank_data(
	void);
#endif

// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
static void get_valid_client_salt(
	unsigned long user_id,
	char *salt,
	short authentication_type);
#endif
// ALAN End

static boolean login_user(
	long user_id, 
	char *password,
	unsigned char *salt,
	short authentication_type,
	boolean *first_time);

static void make_update_buddy_string(
	char * string, 
	char * player_name, 
	boolean add);

static boolean handle_query_motd_packet(
	struct web_client_data * client,
	struct room_packet_header * header);

static boolean handle_change_motd_packet(
	struct web_client_data * client,
	struct room_packet_header * header);

static boolean handle_new_user_packet(
	struct web_client_data * client,
	struct room_packet_header * header);

static boolean handle_change_password_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_create_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_join_order_packet(
	struct web_client_data * client, 
	struct room_packet_header *header);

static boolean handle_leave_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_update_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_query_order_information_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_boot_player_from_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_lock_account_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_unlock_account_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_reset_account_packet(
	struct web_client_data * client, 
	struct room_packet_header * header);

static boolean handle_reset_game_type_packet(
	struct web_client_data *client, 
	struct room_packet_header *header);

static boolean handle_reset_order_packet(
	struct web_client_data *client,
	struct room_packet_header *header);

/* ALAN Begin: this function is declared, but never defined
static boolean handle_reset_order_game_type_packet(
	struct web_client_data *client,
	struct room_packet_header *header);
// ALAN End  */

// _MYTHDEV Begin
#ifdef BN2_FULLVERSION
static int get_build_version_from_player_data( 
	struct player_client_data *player);
#endif
// _MYTHDEV End

static boolean reset_player_scores_by_game_type(short type);
static boolean reset_order_scores_by_game_type(short type);


static boolean b_send_mail = TRUE;

static struct caste_breakpoint_data caste_breakpoints;

// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
	static struct overall_ranking_data overall_rank_data;
#endif
// ALAN End

/* --------- code */
int main(
	int argc,
	char *argv[])
{
    printf("[INFO] userd_new starting up...\n");
    fflush(stdout);
    printf("[INFO] userd_new starting up...\n");
    fflush(stdout);
	setbuf(stdout, NULL); // Disable stdout buffering for immediate log output
	printf("Starting userd_new main()\n"); // Confirm server startup
    parse_command_arguments(argc, argv);

    printf("[INFO] Startup complete — entering daemon mode.\n");
    printf("[INFO] userd_new entering main server loop...\n");
    fflush(stdout);
    run_server();
    printf("[ERROR] userd_new main server loop exited unexpectedly!\n");
    fflush(stdout);
    printf("[INFO] userd_new shutting down.\n");
    fflush(stdout);

	return 0;
}

/* --------- local code */
static void parse_command_arguments(
	int argc, 
	char *argv[])
{
	int opt;

    // [DEBUG] CASCADE HARDCODE TEST - THIS LINE PROVES CODE WAS EDITED
    printf("[DEBUG] CASCADE HARDCODE TEST - userd_port about to be set\n");
    user_parameters.userd_port = 6321; // userd: 6321
    printf("[INFO] userd_port forcibly set to 6321 to match client.\n");
	user_parameters.web_port= atoi(get_userd_web_port());
	user_parameters.room_port = atoi(get_userd_room_port());
	user_parameters.rooms= NULL;
	strcpy(user_parameters.new_user_login, "login");
	strcpy(user_parameters.room_login, "login");
	user_parameters.clients= NULL;
	strcpy(user_parameters.stats_mail_address, "your@email.com");

	// ALAN Begin: a few new startup options
//	while((opt= getopt(argc, argv, "dzhn:r:p:m:")) != -1)
	while((opt= getopt(argc, argv, "uobdzhn:r:p:m:")) != -1)
	// ALAN End
	{
		switch(opt)
		{
			case 'z':
				b_send_mail = FALSE;
				break;
			case 'm':
				printf("Stats mailto address is %s\n", optarg);
				strcpy(user_parameters.stats_mail_address, optarg);
				break;
				
			case 'n':
				printf("New user password is %s\n", optarg);
				strcpy(user_parameters.new_user_login, optarg);
				break;
				
			case 'r':
				printf("Room login password is %s\n", optarg);
				strcpy(user_parameters.room_login, optarg);
				break;
				
			case 'p':
				user_parameters.userd_port= atoi(optarg);
				break;
				
			case 'h':
				printf("Usage: %s [-n new_user_password -r room_login_password -p port -m stats@domain.com]\n", argv[0]);
				exit(0);
				break;
				
			case '?':
				// unrecognized option
				printf("Unrecognized option: %c\n", optopt);
				break;
				
			case ':':
				printf("Option needs a value\n");
				break;

				// ALAN Begin: for initial setup ONLY
			case 'u':
				printf("Creating new users database\n");
				create_user_database();
				break;

			case 'o':
				printf("Creating new orders database\n");
				create_order_database();
				break;

			case 'b':
				printf("creating both new users and new orders databases\n");
				create_user_database();
				create_order_database();
				break;
				// ALAN End
		}
	}

    printf("[INFO] userd_new shutting down.\n");
    fflush(stdout);
    printf("[INFO] userd_new main() returning 0 (normal shutdown).\n");
    fflush(stdout);
    return 0;
}

int init_server(void)
{
    printf("[DEBUG] init_server: entered\n");
    printf("[INFO] [init_server] init_server() starting...\n");

    printf("[DEBUG] init_server: before initialize_user_database()\n");
    if (!initialize_user_database()) {
        fprintf(stderr, "[ERROR] init_server: initialize_user_database() failed\n");
        return 0;
    }
    printf("[DEBUG] init_server: after initialize_user_database()\n");
    printf("[DEBUG] After initialize_user_database()\n");

    printf("[DEBUG] init_server: before initialize_order_database()\n");
    if (!initialize_order_database()) {
        fprintf(stderr, "[ERROR] init_server: initialize_order_database() failed\n");
        return 0;
    }
    printf("[DEBUG] init_server: after initialize_order_database()\n");
    printf("[DEBUG] After initialize_order_database()\n");

    printf("[DEBUG] init_server: before load_room_list()\n");
    user_parameters.rooms = load_room_list();
    if (!user_parameters.rooms) {
        fprintf(stderr, "[ERROR] init_server: load_room_list() failed\n");
        return 0;
    }
    printf("[DEBUG] init_server: after load_room_list()\n");
    printf("[DEBUG] After load_room_list()\n");
    user_parameters.clients = NULL;

    printf("[DEBUG] init_server: before signal(SIGPIPE, SIG_IGN)\n");
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "[ERROR] [%s:%s] signal(SIGPIPE) failed: %s\n", __FILE__, __func__, strerror(errno));
        exit(-1);
    }
    printf("[DEBUG] init_server: after signal(SIGPIPE, SIG_IGN)\n");

    printf("[DEBUG] init_server: before create_listening_socket(userd_port=%d)\n", user_parameters.userd_port);
    userd_globals.server_socket = create_listening_socket(user_parameters.userd_port);
    if (userd_globals.server_socket == NONE) {
        fprintf(stderr, "[ERROR] [%s:%s] create_listening_socket(userd_port=%d) failed: %s\n", __FILE__, __func__, user_parameters.userd_port, strerror(errno));
        exit(-1);
    }
    printf("[DEBUG] init_server: after create_listening_socket(userd_port=%d)\n", user_parameters.userd_port);

    printf("[DEBUG] init_server: before create_listening_socket(room_port=%d)\n", user_parameters.room_port);
    userd_globals.room_socket = create_listening_socket(user_parameters.room_port);
    if (userd_globals.room_socket == NONE) {
        fprintf(stderr, "[ERROR] [%s:%s] create_listening_socket(room_port=%d) failed: %s\n", __FILE__, __func__, user_parameters.room_port, strerror(errno));
        exit(-1);
    }
    printf("[DEBUG] init_server: after create_listening_socket(room_port=%d)\n", user_parameters.room_port);

#ifdef BN2_FULLVERSION
    printf("[DEBUG] init_server: before create_listening_socket(web_port=%d)\n", user_parameters.web_port);
    userd_globals.web_socket = create_listening_socket(user_parameters.web_port);
    if (userd_globals.web_socket == NONE) {
        fprintf(stderr, "[ERROR] [%s:%s] create_listening_socket(web_port=%d) failed: %s\n", __FILE__, __func__, user_parameters.web_port, strerror(errno));
        exit(-1);
    }
    printf("[DEBUG] init_server: after create_listening_socket(web_port=%d)\n", user_parameters.web_port);
#endif

    printf("[DEBUG] init_server: before FD_ZERO/FD_SET setup\n");
    userd_globals.set_size  = 0;
    FD_ZERO(&userd_globals.fds_read);
    FD_SET(userd_globals.server_socket, &userd_globals.fds_read);
    FD_SET(userd_globals.room_socket, &userd_globals.fds_read);
#ifdef BN2_FULLVERSION
    FD_SET(userd_globals.web_socket, &userd_globals.fds_read);
#endif
    userd_globals.set_size = MAX(userd_globals.server_socket + 1, userd_globals.room_socket + 1);
#ifdef BN2_FULLVERSION
    userd_globals.set_size = MAX(userd_globals.set_size, userd_globals.web_socket + 1);
#endif
    printf("[DEBUG] init_server: after FD_ZERO/FD_SET setup\n");

    printf("User server waiting on port %d, %ld known users\n", user_parameters.userd_port, get_user_count());

}


static void handle_incoming_connections(int fd)
{
	struct sockaddr_in remote_address;
	int client_socket, client_len;
	short port, type= 0;
	long host;
	
	if (fd == userd_globals.server_socket)
	{
		type = _player_client_type;
	} 
	else if (fd == userd_globals.room_socket)
	{
		type = _room_client_type;
	} 
#ifdef BN2_FULLVERSION
	else if (fd == userd_globals.web_socket)
	{
		type = _web_client_type;
	}
#endif
	
	client_len = sizeof(struct sockaddr_in);
	client_socket = accept(fd, (struct sockaddr *)&remote_address, &client_len);
	if (client_socket != NONE)
	{
		host = ntohl(remote_address.sin_addr.s_addr);
		port = ntohs(remote_address.sin_port);
		if (valid_remote_host(host, type)) 
		{
			int flags = fcntl(client_socket, F_GETFL, 0);

			FD_SET(client_socket, &userd_globals.fds_read);
			userd_globals.set_size = MAX(userd_globals.set_size, client_socket + 1);
			
			fcntl(client_socket, F_SETFL, O_NONBLOCK|flags);
			user_parameters.clients = add_client(user_parameters.clients, client_socket, host, port, type);
		} 
		else 
		{
			close(client_socket);
		}
	} 
	else
	{
		perror("Client socket was -1 on accept.");
		errno = 0;
	}
}

static void handle_client_connections(int fd)
{
    // Helper for timestamped debug output
    #define PRINT_TIMESTAMPED(fmt, ...) \
        do { \
            time_t rawtime; \
            struct tm * timeinfo; \
            char tbuf[32]; \
            time(&rawtime); \
            timeinfo = localtime(&rawtime); \
            strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", timeinfo); \
            printf("[%s] " fmt "\n", tbuf, ##__VA_ARGS__); \
        } while(0)

	int nread;
	struct client_data * client;

	client = find_client_from_socket(user_parameters.clients, fd);
	if(client) 
	{
		boolean remove_client = FALSE;
					
		ioctl(fd, FIONREAD, &nread);
		if(nread == 0) 
		{
			remove_client = TRUE;
		} 
		else 
		{
			char buffer[MAXIMUM_PACKET_LENGTH];

			if (nread >= sizeof(buffer))
			{
				remove_client = TRUE;
			}
			else
			{
				read(fd, buffer, nread);
                // Debug: Log incoming packet info
                PRINT_TIMESTAMPED("Received %d bytes from fd=%d (client_type=%d, state=%d)", nread, fd, client->type, client->state);
                int debug_len = nread < 32 ? nread : 32;
                printf("    Data (hex, max 32 bytes): ");
                for (int i = 0; i < debug_len; ++i) printf("%02X ", (unsigned char)buffer[i]);
                printf("\n");
				{
					if(copy_data_into_circular_queue(buffer, nread, &client->incoming) == FALSE)
                                {
                                    PRINT_TIMESTAMPED("ERROR: Failed to copy data into circular queue for fd=%d. Disconnecting client.", fd);
                                    remove_client = TRUE;
                                }
				}

				if (remove_client != TRUE)
				{

					switch (client->type)
					{
					case _player_client_type:
						remove_client = handle_client_data((struct player_client_data *)client);
						break;
					case _room_client_type:
							
						remove_client = handle_room_data((struct room_client_data *)client);
						break;
// ALAN Begin: commented this out to eliminate BN2_DEMOVERSION compiler warnings, resolved the issue elsewhere
//#ifdef BN2_FULLVERSION			
					case _web_client_type:
						remove_client = handle_web_client_data((struct web_client_data *)client);
						break;
//#endif
// ALAN End
					default:
						halt();
						break;
					}
				}

			}
		}
		
		if(remove_client) 
        {
            PRINT_TIMESTAMPED("Client on fd=%d (type=%d, state=%d) disconnected.", fd, client->type, client->state);
            send_outgoing_data(client);

            FD_CLR(fd, &userd_globals.fds_read);
            FD_CLR(fd, &userd_globals.fds_write);
            close(client->socket);
            user_parameters.clients= delete_client(user_parameters.clients, client);
        } 
		else if (circular_buffer_size(&client->outgoing))
			FD_SET(fd, &userd_globals.fds_write);

	}
	else
	{
	}
}

static void handle_write_sockets(int fd)
{
	if (FD_ISSET(fd, &userd_globals.fds_write))
	{
		struct client_data * client;

		client = find_client_from_socket(user_parameters.clients, fd);
		if (client) 
		{
			boolean remove_client = FALSE;

			remove_client = send_outgoing_data(client);
			if (remove_client) 
			{
				boolean dead_client_was_room = (client->type == _room_client_type);
			
				FD_CLR(fd, &userd_globals.fds_read);
				close(client->socket);
				user_parameters.clients = delete_client(user_parameters.clients, client);	
				
				if(dead_client_was_room)
				{
					login_next_queued_room(user_parameters.clients);
				}
			} 
		} 
		else 
		{
		}
	}
}

static void run_server(void)
{
	init_server();

	while (1) 
	{
		fd_set fds_test_read;
		int result;

		build_client_writefds(&userd_globals.fds_write);
		fds_test_read = userd_globals.fds_read;

		idle_userd();
		
		build_client_writefds(&userd_globals.fds_write);

		result = select(userd_globals.set_size, &fds_test_read, &userd_globals.fds_write, NULL, NULL);
		if (result<0) 
		{
			exit(-1);
		} 
		else if (result==0) 
		{
			// select timed out
		} 
		else 
		{
			int fd;
			int current_set_size = userd_globals.set_size; 

			for(fd = 0; fd < current_set_size; fd++) 
			{
				if(FD_ISSET(fd, &fds_test_read)) 
				{
					if (fd == userd_globals.server_socket 
						|| fd == userd_globals.room_socket 
						|| fd == userd_globals.web_socket) 
					{
						handle_incoming_connections(fd);
					} 
					else
					{
						handle_client_connections(fd);
					} 
				}
				handle_write_sockets(fd);
			}
		}
	}

	return;
}

/* ALAN Begin: this function defined but not used
static boolean log_room_stats(void)
{
	return TRUE;
}
// ALAN End  */

static boolean valid_remote_host(
	unsigned long host,
	short type)
{
	boolean valid= FALSE;

	switch(type)
	{
		case _web_client_type:
		case _room_client_type:
			if ((host & CLASS_C_NETMASK)==LOCAL_AREA_NETWORK)
			{
				valid= TRUE;
			}
			break;
			
		case _player_client_type:
			valid= TRUE;
			break;
	}
	
	return valid;
}

// 4 hours
#define STATS_EXPORT_PERIOD_IN_SECONDS (4*60*60)

// 5 minutes
#define RERANK_PERIOD_IN_SECONDS (2*60*60)
#define ONE_DAY		(60*60*24)
#define	ONE_MINUTE	(60)

static void idle_userd(
	void)
{
// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
// ALAN End
	static time_t last_stats_export = 0;
	static time_t last_rerank = 0;
	static time_t last_daily_maintenance = 0;
	static time_t one_minute_counter= 0;
	static boolean rerank_completed = FALSE;

	time_t current_time;

	current_time = time(NULL);

// ALAN Begin
// #ifdef BN2_FULLVERSION
// ALAN End
	if (current_time-one_minute_counter>ONE_MINUTE)
	{
		struct room_client_data *room;

		one_minute_counter= current_time;
		room= (struct room_client_data *)user_parameters.clients;
		while (room)
		{
			if(room->type==_room_client_type && room->logged_in==TRUE)
			{
				short length;

				// ALAN Begin: passing arg from incompatible pointer type
			//	length= build_rs_ping_packet(&room->buffer);
				length= build_rs_ping_packet(room->buffer);
				// ALAN End
				add_packet_to_outgoing((struct client_data *)room, room->buffer, length);
			}

			room= (struct room_client_data *)room->next;
		}			
	}
	
	if (!last_stats_export) last_stats_export= current_time;
	if (current_time-last_stats_export>STATS_EXPORT_PERIOD_IN_SECONDS)
	{
		last_stats_export= current_time;
	}

	if ((current_time-last_rerank > RERANK_PERIOD_IN_SECONDS) &&
		!rerank_completed)
	{
		rerank_completed = build_overall_ranking_data(&caste_breakpoints, &overall_rank_data);
		if (rerank_completed)
		{
			last_rerank = current_time;
			rerank_completed = FALSE;
			update_rooms_on_rank_data();
		}
	}

	if ((current_time - last_daily_maintenance) > ONE_DAY)
	{
		struct bungie_net_order_datum order;

		if (get_first_order_information(&order))
		{
			do
			{
				if (get_player_count_in_order(order.order_id) < MINIMUM_NUMBER_OF_PLAYERS_IN_ORDER)
				{
					if (!order.initial_date_below_three_members)
					{
						order.initial_date_below_three_members = time(NULL);
					}
					else
					{
						if ((current_time - order.initial_date_below_three_members) >
							TIME_TO_EXPIRE_ORDER)
						{
							mark_order_as_unused(order.order_id);
						}
					}
				}
			} while (get_next_order_information(&order));
		}

		last_daily_maintenance = current_time;
	}
#endif

	return;
	
}

/* ------------------------------------------- client handling code */

static boolean handle_client_data(	
	struct player_client_data *client)
{
	boolean disconnect_client= FALSE;
	boolean byteswap= FALSE;
	char packet[MAXIMUM_PACKET_LENGTH];
	struct packet_header *header;
	boolean disconn_from_error = FALSE;

#ifdef little_endian
	byteswap= TRUE;
#endif

	header= (struct packet_header *) packet;
	while(!disconnect_client && get_next_packet_from_queue(&client->incoming, header, MAXIMUM_PACKET_LENGTH, byteswap, &disconn_from_error))
	{
		if (disconn_from_error == FALSE)
		{
			// _MYTHDEV Begin
            if ( !decryptData( header, client ) ) {
                disconnect_client = TRUE;
            } else {  
			// _MYTHDEV End

				if(byteswap_packet((char *) header, FALSE))
				{
					switch(header->type) 
					{
                        // _MYTHDEV Begin
					    case _session_key_packet:                                          
                            disconnect_client= handle_session_key_packet(client, header);
						    break;
                        // _MYTHDEV End

						case _login_packet:
							disconnect_client= handle_login_packet(client, header);
							break;	

						case _password_response_packet:
							disconnect_client= handle_password_response_packet(client, header);
							break;
						
						case _version_control_packet:
							disconnect_client= handle_version_control_packet(client, header);
							break;
						
						case _logout_packet:
							disconnect_client= TRUE;
							break;
						
						default:
							disconnect_client= TRUE;
							break;
					}
				 } else {
					vpause(csprintf(temporary, "Unknown packet type from 0x%lx:%d (%d?)\n", client->host, client->port, header->type));
					disconnect_client= TRUE;
				}
			}  // _MYTHDEV
		}		
	}

	if (disconn_from_error == TRUE)
	{
		disconnect_client = TRUE;
	}

	return disconnect_client;
}

// _MYTHDEV Begin
static boolean handle_session_key_packet(  
	struct player_client_data *player_client_data, 
	struct packet_header *packet_header)
{
    char* their_public_key = ((char *) packet_header + sizeof(struct packet_header));  
    boolean disconnect= FALSE;
	short length;
    char my_public_key[ SESSION_KEY_SIZE ];
    char my_private_key[ SESSION_KEY_SIZE ];

    getPublicKey( my_public_key, my_private_key );
    getSessionKey( player_client_data->session_key, my_private_key, their_public_key );

	length= build_sessionkey_packet(player_client_data->buffer, my_public_key , SESSION_KEY_SIZE);
	disconnect= !add_packet_to_outgoing((struct client_data *)player_client_data, player_client_data->buffer, length);
    player_client_data->valid_session_key = TRUE;

    return disconnect;
} 
// _MYTHDEV End

static boolean handle_login_packet(
	struct player_client_data *client, 
	struct packet_header *header)
{
	struct login_data *login = 
		(struct login_data *) ((char *) header + sizeof(struct packet_header));
	boolean disconnect= FALSE;
	short length;
	char login_name[MAXIMUM_LOGIN_LENGTH + 1];

	switch(client->state)
	{
		case _awaiting_login_packet_state:
			client->game_type = game_name_to_type(login->application);
			if(client->game_type != NONE)
			{
				// _MYTHDEV Begin: maintain support for v1.3
				if( client->game_type == 0 ) 
					client->game_type = 2;
				// _MYTHDEV End

				client->platform_type = login->platform_type;
				client->country_code = login->country_code;

				{

// ALAN Begin: modified so we can build DEMOVERSION warning free
#ifdef BN2_FULLVERSION
					struct bungie_net_player_datum player;
#endif
// ALAN End
					strncpy(login_name, login->username, MAXIMUM_LOGIN_LENGTH+1);
					login_name[MAXIMUM_LOGIN_LENGTH] = '\0';

#ifdef BN2_FULLVERSION
					if (get_player_information(login_name, 0, &player))
					{
						if (player.player_is_banned_flag)
						{
							long present_time = time(NULL);

							if ((present_time - player.banned_time) > player.ban_duration)
							{
								player.player_is_banned_flag = FALSE;
								player.banned_time = NONE;
								player.ban_duration = NONE;
								update_player_information(NULL, player.player_id, FALSE, &player);
							}
							else
							{
								disconnect = TRUE;
							}
						}

						player.country_code= login->country_code;

						// _MYTHDEV Begin
						// moved this down, so we update build_version
					//	update_player_information(NULL, player.player_id, FALSE, &player);
						// _MYTHDEV End

						if (!disconnect)
						{
							client->user_id = player.player_id;
							client->order = player.order_index;

							strcpy(client->username, login_name);
							client->authentication_type = MIN(login->max_authentication, NUMBER_OF_ENCRYPTION_TYPES-1);
							get_valid_client_salt(client->user_id, client->salt, client->authentication_type);

							memcpy(client->player_data, login->player_data, login->player_data_size);
							client->player_data_size = login->player_data_size;
							client->reset_player_data = (login->flags & _reset_player_data_flag);

							// _MYTHDEV Begin
							// update database on build_version, even if !client->reset_player_data
							if( !client->reset_player_data )
								player.aux_data.build_version = get_build_version_from_player_data( client );

							// reset game_type_flags
							player.aux_data.game_type_flags = 0;

							// save game_type_flags to database for use in scoring etc..
							switch( client->game_type )
							{
							case 1:
								player.aux_data.game_type_flags |= _myth1_game_type_flag;
								break;
							case 2:
								player.aux_data.game_type_flags |= _myth2_game_type_flag;
								break;
							case 3:
								player.aux_data.game_type_flags |= _myth3_game_type_flag;
								break;
							default:
								// just set them as default M2 I guess.. should never be here
								player.aux_data.game_type_flags |= _myth2_game_type_flag;
								break;
							}

							update_player_information(NULL, player.player_id, FALSE, &player);
							// _MYTHDEV End

							length= build_password_challenge_packet(client->buffer, client->authentication_type, client->salt);
							disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
						}
					} 
#elif defined(BN2_DEMOVERSION)
					strlwr(login->username);
					if (!strcmp(login->username, GUEST_ACCOUNT_NAME))
					{
						get_guest_user_token(client->token);
						client->state= _awaiting_version_packet_state;
						length= build_send_versions_packet(client->buffer);
						client->user_id = 0;
						// ALAN Begin: passing arg 1 from incompatible pointer type fix
					//	disconnect= !add_packet_to_outgoing(client, client->buffer, length);
						disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
						// ALAN End
					}
#endif
					else 
					{
						length= build_server_message_packet(client->buffer, _login_failed_bad_user_or_password);
						disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
					}
#ifdef BN2_FULLVERSION
					client->state= _awaiting_password_response_packet_state;
#endif
				} 
			} else {
				length= build_server_message_packet(client->buffer, _unknown_game_type_msg);
				add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
				disconnect= TRUE;
			}
			break;
			
		case _awaiting_password_response_packet_state:
			length= build_password_challenge_packet(client->buffer, client->authentication_type, client->salt);
			disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
			break;
			
		default:
			break;
	}

	return disconnect;
}

static boolean handle_password_response_packet(
	struct player_client_data *client, 
	struct packet_header *header)
{
	struct password_response_packet * packet= 
		(struct password_response_packet *) ((byte *) header + sizeof(struct packet_header));
	boolean disconnect= FALSE;
	short length;
	boolean first_time;

	switch(client->state)
	{
		case _awaiting_login_packet_state:
			disconnect= TRUE;
			break;
			
		case _awaiting_password_response_packet_state:
			if(login_user(client->user_id, packet->encrypted_password, client->salt, client->authentication_type, &first_time))
			{
				if(first_time)
				{
					client->reset_player_data= TRUE;
				}
			
				generate_authentication_token(client->host, client->user_id, get_current_time()+AUTHENTICATION_EXPIRATION_TIME, client->token);
				client->state= _awaiting_version_packet_state;
				length= build_send_versions_packet(client->buffer);
				disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
				
				{
					struct bungie_net_player_datum player;
					get_player_information(NULL, client->user_id, &player);
					player.last_login_ip_address= client->host;
					update_player_information(NULL, player.player_id, FALSE, &player);
				}
			} else {
				length= build_server_message_packet(client->buffer, _login_failed_bad_user_or_password);
				disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
				if(!disconnect)
				{
					if(++client->login_attempts>=MAXIMUM_BAD_LOGIN_ATTEMPTS) disconnect= TRUE;
				}
			}
			break;
			
		default:
			break;
	}

	return disconnect;
}

static boolean handle_version_control_packet(
	struct player_client_data *client, 
	struct packet_header *header)
{
	boolean disconnect= TRUE;
	
	{
		struct version_control_packet *vcp= (struct version_control_packet *)(header + 1);
		
		if (vcp->patch_number < MINIMUM_PATCH_VERSION_REQUIRED) return TRUE;
	}

	switch(client->state)
	{
		case _awaiting_login_packet_state:
			break;

		case _awaiting_password_response_packet_state:
			break;
			
		case _awaiting_version_packet_state:
			{
				send_successful_login_sequence(client);
				disconnect = FALSE;
			}
			break;
			
		default:
			break;
	}

	return disconnect;
}

#ifdef BN2_FULLVERSION
static void update_rooms_on_rank_data(
	void)
{
	short length;
	char buffer[4096];

	length = build_rs_rank_update_packet(buffer, &caste_breakpoints, &overall_rank_data);
	send_room_packet_to_all_connected_rooms(buffer, length);
}
#endif

static void send_successful_login_sequence(
	struct player_client_data *client)
{
	int length;
	struct room_client_data *room= (struct room_client_data *) user_parameters.clients;
	boolean disconnect;
	boolean allow_all_rooms = FALSE;
	struct bungie_net_player_datum player;

	length= build_user_successful_login_packet(client->buffer, client->user_id, client->order, &client->token);
	disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
	if(!disconnect)
	{
		length= build_game_specific_login_packet(client);
		if(length)
		{
			disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
		}

		if(!disconnect)
		{
			struct player_room_list_data available_rooms[MAXIMUM_ROOMS];
			short available_room_count= 0;
			short client_caste, index;
			
			client_caste= NONE;
			if(client->user_id)
			{
				struct player_stats changed_game_data;
				get_player_information(NULL, client->user_id, &player);

				if(get_user_game_data(client->user_id, client->game_type, &changed_game_data, NULL))
				{	
					long score;
				
					score= convert_player_stats_to_score(&changed_game_data);
				}
			}
			
			while(room)
			{
				if(room->type==_room_client_type && room->logged_in==TRUE)
				{
					struct room_info info;
					
					info.room_id= room->room_identifier;
					info.player_count= room->player_count;

					// _MYTHDEV Begin: host is set to localhost when running locally so external connections don't work					
					#ifdef RUNNING_LOCALLY
						info.host = 0x7f000001;
					#else
						info.host= room->host;
					#endif
					// _MYTHDEV End

					info.port= room->room_port;
					info.game_count= room->game_count;
					info.room_type= room->tournament_room ? _tournament_room : room->ranked_room 
						? _ranked_room : _unranked_room;
						
					available_rooms[available_room_count].info= info;
					available_rooms[available_room_count].country_code= room->country_code;
					available_rooms[available_room_count].minimum_caste= room->minimum_caste;
					available_rooms[available_room_count].maximum_caste= room->maximum_caste;
					available_rooms[available_room_count].tournament_room = room->tournament_room;
					available_room_count+= 1;
				}
				room= (struct room_client_data *) room->next;
			}

			if (player.administrator_flag || (player.special_flags & _bungie_employee_flag))
			{
				allow_all_rooms = TRUE;
			}

			available_room_count= find_client_available_rooms(player.ranked_score.rank, client->country_code, client->username,
				available_rooms, available_room_count, allow_all_rooms);			

			length= start_building_room_packet(client->buffer);
			for(index= 0; index<available_room_count; ++index)
			{
				length= add_room_data_to_packet(client->buffer, &available_rooms[index].info);
			}
			
			disconnect= !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
		}
	}
	
	return;
}

static boolean handle_web_client_data(
	struct web_client_data * client)
{
	boolean disconnect_client = FALSE;
	char packet[MAXIMUM_PACKET_LENGTH];
	struct room_packet_header * header;
	boolean byteswap = FALSE;
	boolean disconn_from_error = FALSE;

	header = (struct room_packet_header *)packet;

// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_DEMOVERSION
	return FALSE;
#endif
// ALAN End

	while(!disconnect_client && 
		get_next_room_packet_from_queue(&client->incoming, header, 
		MAXIMUM_PACKET_LENGTH, byteswap, &disconn_from_error))
	{
		switch (header->type) 
		{
			case _query_motd_packet:
				disconnect_client = handle_query_motd_packet(client, header);
				break;

			case _change_motd_packet:
				disconnect_client = handle_change_motd_packet(client, header);
				break;

			case _new_user_packet:
				disconnect_client = handle_new_user_packet(client, header);
				break;

			case _change_password_packet:
				disconnect_client = handle_change_password_packet(client, header);
				break;

			case _create_order_packet:
				disconnect_client = handle_create_order_packet(client, header);
				break;

			case _join_order_packet:
				disconnect_client = handle_join_order_packet(client, header);
				break;

			case _leave_order_packet:
				disconnect_client = handle_leave_order_packet(client, header);
				break;

			case _update_order_information_packet:
				disconnect_client = handle_update_order_packet(client, header);
				break;

			case _query_order_information_packet:
				disconnect_client = handle_query_order_information_packet(client, header);
				break;

			case _boot_player_from_order_packet:
				disconnect_client = handle_boot_player_from_order_packet(client, header);
				break;

			case _lock_account_packet:
				disconnect_client = handle_lock_account_packet(client, header);
				break;

			case _unlock_account_packet:
				disconnect_client = handle_unlock_account_packet(client, header);
				break;

			case _reset_account_packet:
				disconnect_client = handle_reset_account_packet(client, header);
				break;

			case _reset_game_type_packet:
				disconnect_client = handle_reset_game_type_packet(client, header);
				break;

			case _reset_order_packet:
				disconnect_client= handle_reset_order_packet(client, header);
				break;
		}
	}

	return disconnect_client;
}


static boolean handle_query_motd_packet(
	struct web_client_data * client,
	struct room_packet_header * header)
{
	short length;

	length = build_web_server_response_packet(client->buffer, _query_motd_response_packet, _success, strlen(user_parameters.motd) + 1, user_parameters.motd);
	return !(add_packet_to_outgoing((struct client_data *)client, client->buffer, length));
}

static boolean handle_change_motd_packet(
	struct web_client_data * client,
	struct room_packet_header * header)
{
	short length;
	char * new_motd;

	new_motd = (char *)header;
	new_motd += sizeof(struct room_packet_header);

	handle_change_motd(new_motd);

	length = build_web_server_response_packet(client->buffer, _change_motd_response_packet, _success, 0, "");
	return !(add_packet_to_outgoing((struct client_data *)client, client->buffer, length));
}

static boolean handle_new_user_packet(
	struct web_client_data * client,
	struct room_packet_header * header)
{
	short length, n;
	char * string;

	struct bungie_net_player_datum player;

	string = (char *)header;
	string += sizeof(struct room_packet_header);

	memset(&player, 0, sizeof(struct bungie_net_player_datum));
	strncpy(player.login, string, MAXIMUM_LOGIN_LENGTH + 1);
	player.login[MAXIMUM_LOGIN_LENGTH] = '\0';

	length = strlen(player.login) + 1;
	string += length;
	strncpy(player.password, string, MAXIMUM_PASSWORD_LENGTH + 1);
	player.password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	player.order_index = 0;

	for (n = 0; n < MAXIMUM_BUDDIES; ++n)
	{
		player.buddies[n].player_id = 0;
		player.buddies[n].active = INACTIVE;
	}

	player.unranked_score.numerical_rank = get_user_count() + 1;
	player.ranked_score.numerical_rank = get_user_count() + 1;
	for (n = 0; n < MAXIMUM_NUMBER_OF_GAME_TYPES; ++n)
	{
		player.ranked_scores_by_game_type[n].numerical_rank = get_user_count() + 1;
	}

	if (new_user(&player))
	{
		length = build_web_server_response_packet(client->buffer, _new_user_response_packet, _success, sizeof(unsigned long), (void *)&player.player_id);
	}
	else
	{
		player.player_id = NONE;
		length = build_web_server_response_packet(client->buffer, _new_user_response_packet, _failure, sizeof(unsigned long), (void *)&player.player_id);
	}

	return !(add_packet_to_outgoing((struct client_data *)client, client->buffer, length));
}

static boolean handle_change_password_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	short length;
	char *p;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	char new_password[MAXIMUM_PASSWORD_LENGTH + 1];
	char error_string[128];

	struct bungie_net_player_datum player;

	boolean success = FALSE;

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(login, p, MAXIMUM_LOGIN_LENGTH + 1);
	login[MAXIMUM_LOGIN_LENGTH] = '\0';

	length= strlen(login) + 1;
	p+= length;

	strncpy(new_password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	new_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	if (get_player_information(login, 0, &player))
	{
		strcpy(player.password, new_password);
		update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
		success = TRUE;
	}
	else
	{
		sprintf(error_string, "The login %s is invalid.", login);
	}

	if (success)
	{
		length = build_web_server_response_packet(client->buffer, _change_password_response_packet, _success, 0, "");
	}
	else
	{
		length = build_web_server_response_packet(client->buffer, _change_password_response_packet, _failure, strlen(error_string) + 1, error_string);
	}

	return !(add_packet_to_outgoing((struct client_data *)client, client->buffer, length));
}

static boolean handle_create_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	short length, n;
	char * p;
	struct bungie_net_order_datum order;

	memset(&order, 0, sizeof(struct bungie_net_order_datum));

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	strncpy(order.name, p, MAXIMUM_ORDER_NAME_LENGTH + 1);
	order.name[MAXIMUM_ORDER_NAME_LENGTH] = '\0';
	
	length = strlen(order.name) + 1;
	p += length;
	strncpy(order.maintenance_password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	order.maintenance_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	length = strlen(order.maintenance_password) + 1;
	p += length;
	strncpy(order.member_password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	order.member_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	length = strlen(order.member_password) + 1;
	p += length;
	strncpy(order.url, p, MAXIMUM_ORDER_URL_LENGTH + 1);
	order.url[MAXIMUM_ORDER_URL_LENGTH] = '\0';

	length = strlen(order.url) + 1;
	p += length;
	strncpy(order.contact_email, p, MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH + 1);
	order.contact_email[MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH] = '\0';

	length = strlen(order.contact_email) + 1;
	p += length;
	strncpy(order.motto, p, MAXIMUM_ORDER_MOTTO_LENGTH + 1);
	order.motto[MAXIMUM_ORDER_MOTTO_LENGTH] = '\0';

	order.unranked_score.numerical_rank = get_order_count() + 1;
	order.ranked_score.numerical_rank = get_order_count() + 1;
	for (n = 0; n < MAXIMUM_NUMBER_OF_GAME_TYPES; ++n)
	{
		order.ranked_scores_by_game_type[n].numerical_rank = get_order_count() + 1;
	}

	if (new_order(&order))
	{
		length = build_web_server_response_packet(client->buffer, _create_order_response_packet, _success, 0, "");
	}
	else
	{
		char failure_string[256];

		sprintf(failure_string, "order already exists");
		length = build_web_server_response_packet(client->buffer, _create_order_response_packet, _failure, strlen(failure_string)+1, failure_string);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_join_order_packet(
	struct web_client_data * client, 
	struct room_packet_header *header)
{
	char * p;
	short length;
	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];
	char member_password[MAXIMUM_PASSWORD_LENGTH + 1];

	char login[MAXIMUM_LOGIN_LENGTH + 1];
	char password[MAXIMUM_PASSWORD_LENGTH + 1];

	struct bungie_net_player_datum player;
	struct bungie_net_order_datum order;

	boolean success = FALSE;

	char failure_string[256];

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	strncpy(order_name, p, MAXIMUM_ORDER_NAME_LENGTH + 1);
	order_name[MAXIMUM_ORDER_NAME_LENGTH] = '\0';

	length = strlen(order_name) + 1;
	p += length;
	strncpy(member_password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	member_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	length = strlen(member_password) + 1;
	p += length;
	strncpy(login, p, MAXIMUM_LOGIN_LENGTH + 1);
	login[MAXIMUM_LOGIN_LENGTH] = '\0';

	length = strlen(login) + 1;
	p += length;
	strncpy(password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	if (get_player_information(login, 0, &player))
	{
		if (!strcmp(password, player.password))
		{
			if (get_order_information(order_name, 0, &order))
			{
				if (get_player_count_in_order(order.order_id)<MAXIMUM_ORDER_MEMBERS)
				{
					if (!strcmp(order.member_password, member_password))
					{
						player.order_index = order.order_id;
						update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
						success = TRUE;
					}
					else
					{
						sprintf(failure_string, "the member password is incorrect");
					}
				}
				else
				{
					sprintf(failure_string, "there are already %d members in the order", MAXIMUM_ORDER_MEMBERS);
				}
			}
			else
			{
				sprintf(failure_string, "order does not exist");
			}
		}
		else
		{
			sprintf(failure_string, "player password is incorrect");
		}
	}
	else
	{
		sprintf(failure_string, "login does not exist");
	}

	if (success)
	{
		length = build_web_server_response_packet(client->buffer, _join_order_response_packet, _success, 0, "");
	}
	else
	{
		length = build_web_server_response_packet(client->buffer, _join_order_response_packet, _failure, strlen(failure_string)+1, failure_string);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_leave_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char *p;
	short length;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	char password[MAXIMUM_PASSWORD_LENGTH + 1];
	struct bungie_net_player_datum player;
	boolean success= FALSE;
	char *failure_reason= "";

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(login, p, MAXIMUM_LOGIN_LENGTH + 1);
	login[MAXIMUM_LOGIN_LENGTH] = '\0';
	
	length= strlen(login) + 1;
	p+= length;
	strncpy(password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	if (get_player_information(login, 0, &player))
	{
		if (!strcmp(password, player.password))
		{
			player.order_index = 0;
			update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
			success = TRUE;
		}
		else
		{
			failure_reason= "incorrect password";
		}
	}
	else
	{
		failure_reason= "could not find any such player";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _leave_order_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _leave_order_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_update_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char *p;
	short length;
	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];
	char maintenance_password[MAXIMUM_PASSWORD_LENGTH + 1];
	char member_password[MAXIMUM_PASSWORD_LENGTH + 1];
	char url[MAXIMUM_ORDER_URL_LENGTH + 1];
	char contact_email[MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH + 1];
	char motto[MAXIMUM_ORDER_MOTTO_LENGTH + 1];
	char *failure_reason= "";

	struct bungie_net_order_datum order;

	boolean success= FALSE;

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(order_name, p, MAXIMUM_ORDER_NAME_LENGTH + 1);
	order_name[MAXIMUM_ORDER_NAME_LENGTH] = '\0';

	length= strlen(order_name) + 1;
	p+= length;
	strncpy(maintenance_password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	maintenance_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	length= strlen(maintenance_password) + 1;
	p+= length;
	strncpy(member_password, p, MAXIMUM_PASSWORD_LENGTH + 1);
	member_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	length= strlen(member_password) + 1;
	p+= length;
	strncpy(url, p, MAXIMUM_ORDER_URL_LENGTH + 1);
	url[MAXIMUM_ORDER_URL_LENGTH] = '\0';

	length= strlen(url) + 1;
	p+= length;
	strncpy(contact_email, p, MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH + 1);
	contact_email[MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH] = '\0';

	length= strlen(contact_email) + 1;
	p+= length;
	strncpy(motto, p, MAXIMUM_ORDER_MOTTO_LENGTH + 1);
	motto[MAXIMUM_ORDER_MOTTO_LENGTH] = '\0';

	if (get_order_information(order_name, 0, &order))
	{
		if (!strcmp(order.maintenance_password, maintenance_password))
		{
			strcpy(order.member_password, member_password);
			strcpy(order.url, url);
			strcpy(order.contact_email, contact_email);
			strcpy(order.motto, motto);
			update_order_information(NULL, order.order_id, &order);
			success= TRUE;
		}
		else
		{
			failure_reason= "incorrect order password";
		}
	}
	else
	{
		failure_reason= "could not find an order by that name";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _update_order_information_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _update_order_information_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);	
}

static boolean handle_query_order_information_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char * p;
	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];
	char buffer[MAXIMUM_PASSWORD_LENGTH + 1 + MAXIMUM_ORDER_URL_LENGTH + 1 + MAXIMUM_ORDER_CONTACT_EMAIL_LENGTH + 1 + MAXIMUM_ORDER_MOTTO_LENGTH + 1];
	char *failure_reason= "";
	short length, data_length= 0;

	boolean success= FALSE;

	struct bungie_net_order_datum order;

	p= (char *)header;
	p+= sizeof(struct room_packet_header);
	strncpy(order_name, p, MAXIMUM_ORDER_NAME_LENGTH + 1);
	order_name[MAXIMUM_ORDER_NAME_LENGTH] = '\0';

	if (get_order_information(order_name, 0, &order))
	{
		p= (char *)buffer;
		data_length= 0;

		strcpy(p, order.member_password);
		data_length+= strlen(order.member_password) + 1;
		p+= strlen(order.member_password) + 1;

		strcpy(p, order.url);
		data_length+= strlen(order.url) + 1;
		p+= strlen(order.url) + 1;

		strcpy(p, order.contact_email);
		data_length+= strlen(order.contact_email) + 1;
		p+= strlen(order.contact_email);

		strcpy(p, order.motto);
		data_length+= strlen(order.motto) + 1;

		success= TRUE;
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _query_order_information_response_packet, _success, data_length, buffer);
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _query_order_information_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);	
}

static boolean handle_boot_player_from_order_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char *p;
	short length;

	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];
	char maintenance_password[MAXIMUM_PASSWORD_LENGTH + 1];
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	char *failure_reason= "";
	struct bungie_net_player_datum player;
	struct bungie_net_order_datum order;

	boolean success= FALSE;

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(order_name, p, MAXIMUM_ORDER_NAME_LENGTH + 1);
	order_name[MAXIMUM_ORDER_NAME_LENGTH] = '\0';

	length= strlen(order_name) + 1;
	p+= length;
	strncpy(maintenance_password, p, MAXIMUM_PASSWORD_LENGTH);
	maintenance_password[MAXIMUM_PASSWORD_LENGTH] = '\0';

	length= strlen(maintenance_password) + 1;
	p+= length;
	strncpy(login, p, MAXIMUM_LOGIN_LENGTH);
	login[MAXIMUM_LOGIN_LENGTH] = '\0';
	if (get_order_information(order_name, 0, &order))
	{
		if (!strcmp(order.maintenance_password, maintenance_password))
		{
			if (get_player_information(login, 0, &player))
			{
				if (player.order_index == order.order_id)
				{
					player.order_index = 0;
					update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
					success= TRUE;
				}
				else
				{
					failure_reason= "that player is not in your order!";
				}
			}
			else
			{
				failure_reason= "could not find a player with that login name";
			}
		}
		else
		{
			failure_reason= "incorrect order maintenance password";
		}
	}
	else
	{
		failure_reason= "could not find an order with that name";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _boot_player_from_order_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _boot_player_from_order_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);	
}

static boolean handle_lock_account_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char *p;
	struct lock_account_packet * packet;
	struct bungie_net_player_datum player;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	boolean success = FALSE;
	short length;
	char *failure_reason= "";

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	packet= (struct lock_account_packet *)p;
	p+= sizeof(struct lock_account_packet);

	strncpy(login, p, MAXIMUM_LOGIN_LENGTH + 1);
	login[MAXIMUM_LOGIN_LENGTH] = '\0';

	if (get_player_information(login, 0, &player))
	{
		if (packet->ban_duration > 0)
		{
			player.player_is_banned_flag = TRUE;
			player.banned_time = time(NULL);
			player.ban_duration = packet->ban_duration;
			player.times_banned++;
			update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
			success= TRUE;
		}
		else
		{
			failure_reason= "invalid ban duration";
		}	
	}
	else
	{
		failure_reason= "could not find a player with that login name";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _lock_account_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _lock_account_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_unlock_account_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char *p;
	struct bungie_net_player_datum player;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	boolean success= FALSE;
	short length;
	char *failure_reason= "";

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(login, p, MAXIMUM_LOGIN_LENGTH + 1);
	login[MAXIMUM_LOGIN_LENGTH]= '\0';

	if (get_player_information(login, 0, &player))
	{
		player.player_is_banned_flag= FALSE;
		player.banned_time= 0;
		player.ban_duration= 0;
		update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
		success= TRUE;
	}
	else
	{
		failure_reason= "could not find a player with that login name";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _unlock_account_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _unlock_account_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_reset_account_packet(
	struct web_client_data * client, 
	struct room_packet_header * header)
{
	char *p;
	struct bungie_net_player_datum player;
	char login[MAXIMUM_LOGIN_LENGTH + 1];
	boolean success= FALSE;
	short length;
	char *failure_reason= "";

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(login, p, MAXIMUM_LOGIN_LENGTH + 1);
	login[MAXIMUM_LOGIN_LENGTH]= '\0';

	if (get_player_information(login, 0, &player))
	{
		memset(&player.unranked_score, 0, sizeof(struct bungie_net_player_score_datum));
		memset(&player.ranked_score, 0, sizeof(struct bungie_net_player_score_datum));
		memset(&player.ranked_scores_by_game_type, 0, MAXIMUM_NUMBER_OF_GAME_TYPES * sizeof(struct bungie_net_player_score_datum));
		update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
		success = TRUE;
	}
	else
	{
		failure_reason= "could not find a player with that login name";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _unlock_account_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _unlock_account_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_reset_game_type_packet(
	struct web_client_data *client, 
	struct room_packet_header *header)
{
	struct reset_game_type_packet *packet;
	char *p;
	boolean success= FALSE;
	short length;

	p= (char *)header;
	p+= sizeof(struct room_packet_header);
	packet= (struct reset_game_type_packet *)p;

	if (packet->order)
		success= reset_order_scores_by_game_type(packet->type);
	else
		success= reset_player_scores_by_game_type(packet->type);

	if (success)
		length= build_web_server_response_packet(client->buffer, _reset_game_type_response_packet, _success, 0, "");
	else
		length= build_web_server_response_packet(client->buffer, _reset_game_type_response_packet, _failure, 0, "");

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}

static boolean handle_reset_order_packet(
	struct web_client_data *client,
	struct room_packet_header *header)
{
	char *p;
	struct bungie_net_order_datum order;
	char order_name[MAXIMUM_ORDER_NAME_LENGTH + 1];
	boolean success= FALSE;
	short length;
	char *failure_reason= "";

	p= (char *)header;
	p+= sizeof(struct room_packet_header);

	strncpy(order_name, p, MAXIMUM_ORDER_NAME_LENGTH + 1);
	order_name[MAXIMUM_ORDER_NAME_LENGTH] = '\0';

	if (get_order_information(order_name, 0, &order))
	{
		memset(&order.unranked_score, 0, sizeof(struct bungie_net_player_score_datum));
		memset(&order.ranked_score, 0, sizeof(struct bungie_net_player_score_datum));
		memset(&order.ranked_scores_by_game_type, 0, MAXIMUM_NUMBER_OF_GAME_TYPES * sizeof(struct bungie_net_player_score_datum));
		update_order_information(NULL, order.order_id, &order);
		success = TRUE;
	}
	else
	{
		failure_reason= "could not find an order with that name";
	}

	if (success)
	{
		length= build_web_server_response_packet(client->buffer, _unlock_account_response_packet, _success, 0, "");
	}
	else
	{
		length= build_web_server_response_packet(client->buffer, _unlock_account_response_packet, _failure, strlen(failure_reason)+1, failure_reason);
	}

	return !add_packet_to_outgoing((struct client_data *)client, client->buffer, length);
}


/* ------------------------------------------- room handling code */
static boolean handle_rs_login_packet(struct room_client_data *client, struct room_packet_header *header);
static boolean handle_rs_update_ranking_packet(struct room_client_data *client, struct room_packet_header *header);
static boolean handle_rs_update_room_data_packet(struct room_client_data *client, struct room_packet_header *header);
static boolean handle_rs_request_ranking_packet(struct room_client_data *client,  struct room_packet_header *header);
static boolean handle_rs_status_packet(struct room_client_data *client,  struct room_packet_header *header);
static boolean handle_rs_public_announcement_packet(struct room_client_data * client, struct room_packet_header * header);
static boolean handle_rs_player_information_query_packet (struct room_client_data *client, struct room_packet_header *header);
static boolean handle_rs_update_buddy_packet(struct room_client_data * client, struct room_packet_header * header);

static short find_next_available_room(short identifier_hint, short *game_type, short *ranked_room, short * tournament_room,
	short *country_code, short *minimum_caste, short *maximum_caste);
static void mark_room_as_available(short game_type, short room_identifier);

static boolean handle_room_data(
	struct room_client_data *client)
{
	boolean disconnect_client= FALSE;
	char packet[MAXIMUM_PACKET_LENGTH];
	struct room_packet_header *header;
	boolean byteswap= FALSE;
	boolean disconn_from_error = FALSE;

#ifdef little_endian
	byteswap= TRUE;
#endif

	header= (struct room_packet_header *) packet;
	while(!disconnect_client && 
		get_next_room_packet_from_queue(&client->incoming, header, 
		MAXIMUM_PACKET_LENGTH, byteswap, &disconn_from_error))
	{
		disconnect_client= !byteswap_room_packet((char *) header, FALSE);

		if (!disconnect_client)
		{
			switch(header->type) 
			{
				case _rs_login_packet:
					disconnect_client= handle_rs_login_packet(client, header);
					break;

				case _rs_player_information_query_packet:
					disconnect_client = handle_rs_player_information_query_packet(client, header);
					break;

				case _rs_public_announcement_packet:
					disconnect_client = handle_rs_public_announcement_packet(client, header);
					break;	
				
				case _rs_update_ranking_packet:
					disconnect_client= handle_rs_update_ranking_packet(client, header);
					break;
		
				case _rs_update_room_data_packet:
					disconnect_client= handle_rs_update_room_data_packet(client, header);
					break;
				
				case _rs_request_ranking_packet:
					disconnect_client= handle_rs_request_ranking_packet(client, header);
					break;
					
				case _rs_status_packet:
					disconnect_client= handle_rs_status_packet(client, header);
					break;

				case _rs_update_buddy_packet:
					disconnect_client = handle_rs_update_buddy_packet(client, header);
					break;
	

				case _rs_player_enter_room_packet:
					disconnect_client = handle_rs_player_enter_room_packet(client, header);
					break;

				case _rs_player_leave_room_packet:
					disconnect_client = handle_rs_player_leave_room_packet(client, header);
					break;

				case _rs_player_query_packet:
					disconnect_client = handle_rs_player_query_packet(client, header);
					break;

				case _rs_update_player_information_packet:
					disconnect_client = handle_rs_update_player_information_packet(client, header);
					break;

	
				case _rs_player_info_request_packet:
					disconnect_client = handle_rs_player_info_request_packet(client, header);
					break;

				case _rs_ban_player_packet:
					disconnect_client = handle_rs_ban_player_packet(client, header);
					break;

	
				case _rs_score_game_packet:
#ifdef BN2_DEMOVERSION
					disconnect_client = FALSE;
#else
					disconnect_client = handle_rs_score_game_packet(client, header);
#endif
					break;

				default:
					disconnect_client= TRUE;
					break;
			}
		}
	}

	if (disconn_from_error == TRUE)
	{
		disconnect_client = TRUE;
	}

	return disconnect_client;
}


// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
static boolean handle_rs_score_game_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	struct rs_score_game_packet * packet;
	char * p = (char *)header;

	boolean success = TRUE;

	int n, t;
	boolean update_order;

	struct bungie_net_player_datum players[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	struct bungie_net_order_datum orders[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	struct bungie_net_game_standings * standings[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	struct bungie_net_player_datum * p_players[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	struct bungie_net_order_datum * p_orders[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	unsigned long updated_orders[MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME];
	short updated_order_index;

	p += sizeof(struct room_packet_header);
	packet = (struct rs_score_game_packet *)p;

	for (n = 0; n < packet->player_count; ++n)
	{
		if (!get_player_information(NULL, packet->players[n].player_id, &players[n]))
		{
			success = FALSE;
			printf("player not in database, player id = %u\n", packet->players[n].player_id);
			break;
		}

		p_players[n] = &players[n];

		if (packet->game_classification == _unranked_order ||
			packet->game_classification == _ranked_order ||
			packet->game_classification == _tournament_order)
		{
			if (!get_order_information(NULL, players[n].order_index, &orders[n]))
			{
				success = FALSE;
				printf("order not in databse, player id = %u\n; order id = %u\n",
					players[n].player_id, players[n].order_index);
				break;
			}
			p_orders[n] = &orders[n];
		}
		else
		{
			p_orders[n] = NULL;
		}

		if (packet->players[n].reported_standings)
		{
			standings[n] = &packet->players[n].standings;
		}
		else
		{
			standings[n] = NULL;
		}
	}

	if (success)
	{
		bungie_net_game_evaluate(packet->creator_player_id, packet->game_classification, 
			packet->player_count, (struct bungie_net_player_datum **)p_players, 
			p_orders, standings);
	
		updated_order_index = 0;
		memset(&updated_orders[0], 0, sizeof(unsigned long) * MAXIMUM_PLAYERS_PER_METASERVER_HOSTED_GAME);
		for (n = 0; n < packet->player_count; ++n)
		{
			if (packet->game_classification == _unranked_order ||
				packet->game_classification == _ranked_order ||
				packet->game_classification == _tournament_order)
			{
				update_order = TRUE;
				for (t = 0; t < updated_order_index; ++t)
				{
					if (players[n].order_index == updated_orders[t])
					{
						update_order = FALSE;
						break;
					}
				}

				if (update_order)
				{
					update_order_information(NULL, players[n].order_index, &orders[n]);
					updated_orders[updated_order_index++] = players[n].order_index;
				}
			}
			else
			{
				update_player_information(NULL, players[n].player_id, is_player_online(players[n].player_id), &players[n]);
			}
		}

		for (n = 0; n < packet->player_count; ++n)
		{
			struct player_stats stats;
			short user_flags;
	
			if(get_user_game_data(players[n].player_id, 0, &stats, &user_flags))
			{
				short length, flags;

				// ALAN Begin: unused variable, and unused function..
			//	long score = convert_player_stats_to_score(&stats);
				// ALAN End

				struct room_client_data * room_client = find_room_client_from_room_id(players[n].room_id);
			
				if (room_client)
				{
					flags= 0;
					length= build_rs_client_ranking_packet(room_client->buffer, players[n].player_id, flags, &stats);
					add_room_packet_to_outgoing(room_client, room_client->buffer, length);
				}
			}
		}
	}

	return FALSE;
}
#endif
// ALAN End

static boolean handle_rs_ban_player_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	struct rs_ban_player_packet * packet;
	
	char * p = (char *)header;
	struct bungie_net_player_datum player;

	p += sizeof(struct room_packet_header);
	packet = (struct rs_ban_player_packet *)p;

	get_player_information(NULL, packet->player_id, &player);
	player.player_is_banned_flag = TRUE;
	player.banned_time = time(NULL);
	player.ban_duration = packet->ban_duration;
	player.times_banned++;

	update_player_information(NULL, packet->player_id, FALSE, &player);

	return FALSE;
}

static boolean handle_rs_player_information_query_packet (
	struct room_client_data *client, 
	struct room_packet_header *header)
{
	struct rs_player_information_query_packet * packet;
	short packet_length;
	char * p = (char *)header;
	struct bungie_net_player_datum player;
	
	struct buddy_entry buddies[MAXIMUM_BUDDIES];
	short order;
	short buddy_index;

	p += sizeof(struct room_packet_header);
	packet = (struct rs_player_information_query_packet *)p;

	get_player_information(NULL, packet->player_id, &player);
	memcpy(buddies, player.buddies, sizeof(buddies));

	for (buddy_index = 0; buddy_index < MAXIMUM_BUDDIES; buddy_index++)
	{
		if (buddies[buddy_index].player_id > 0 &&
		    buddies[buddy_index].player_id <= get_user_count())
		{
			if (!is_player_online(buddies[buddy_index].player_id))
			{
				buddies[buddy_index].active = OFFLINE;
			}
		}
		else
		{
			break;
		}
	}

	order = player.order_index;

	packet_length = build_rs_player_information_packet(client->buffer, packet->player_id, &buddies[0], order,
		player.administrator_flag, player.special_flags & _bungie_employee_flag, player.special_flags & _account_is_kiosk_flag, player.country_code, player.login);
	add_room_packet_to_outgoing(client, client->buffer, packet_length);

	return FALSE;
}

static boolean handle_rs_login_packet(
	struct room_client_data *client, 
	struct room_packet_header *header)
{
	struct rs_login_packet *login= (struct rs_login_packet *) ((char *) header + sizeof(struct room_packet_header));
	boolean disconnect= FALSE;
	short length;

	if(strcmp(login->password, user_parameters.room_login)==0)
	{
		client->room_port= login->port;
		if (try_to_login_client_room(client, login->identifier))
		{
			if (send_room_list_to_rooms(TRUE) == FALSE)
			{
				// oh well
			}
			else
			{
#ifdef BN2_FULLVERSION
				update_rooms_on_rank_data();
#endif
			}
		}
	}
	else
	{
		length= build_rs_login_failure_packet(client->buffer, _room_bad_password_error_code);
		add_room_packet_to_outgoing(client, client->buffer, length);
		disconnect= TRUE;
	}

	return disconnect;
}

static boolean handle_rs_public_announcement_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	send_room_packet_to_all_connected_rooms((char *)header, header->length);

	return FALSE;
}

static void make_update_buddy_string(
	char * string, 
	char * player_name, 
	boolean add)
{
	if (add)
	{
		sprintf(string, "%s wants to be your buddy!", player_name);
	}
	else
	{
		sprintf(string, "%s no longer wants to be your buddy ...", player_name);
	}
}

static boolean handle_rs_update_buddy_packet(
	struct room_client_data * client, 
	struct room_packet_header * header)
{
	char * p;
	struct buddy_entry buddies[MAXIMUM_BUDDIES];
	char player_name[BUFSIZ];
	char temp[BUFSIZ];
	short o;
	struct rs_update_buddy_packet * packet;
	struct bungie_net_player_datum player;
	short bi;

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_update_buddy_packet *)p;
	update_buddy_list(packet->player_id, packet->buddy_id, packet->add);

	get_player_information(NULL, packet->player_id, &player);
	sprintf(player_name, "%s(%s)", player.name, player.login);

	memcpy(buddies, player.buddies, sizeof(buddies));

	for (bi = 0; bi < MAXIMUM_BUDDIES; bi++)
	{
		if (buddies[bi].player_id)
		{
			if (!is_player_online(buddies[bi].player_id))
			{
				buddies[bi].active = OFFLINE;
			}
		}
		else
		{
			break;
		}
	}

	o = build_rs_update_buddy_response_packet(client->buffer, packet->player_id, buddies);
	add_room_packet_to_outgoing(client, client->buffer, o);

	get_player_information(NULL, packet->buddy_id, &player);

	client = find_room_client_from_room_id(player.room_id);
	if (client && is_player_online(player.player_id))
	{
		memcpy(buddies, player.buddies, sizeof(buddies));
		for (bi = 0; bi < MAXIMUM_BUDDIES; bi++)
		{
			if (buddies[bi].player_id)
			{
				if (!is_player_online(buddies[bi].player_id))
				{
					buddies[bi].active = OFFLINE;
				}
			}
			else
			{
				break;
			}
		}

		o = build_rs_update_buddy_response_packet(client->buffer, packet->buddy_id, buddies);
		add_room_packet_to_outgoing(client, client->buffer, o);

		make_update_buddy_string(temp, player_name, packet->add);
		o = build_rs_global_message_packet(client->buffer, packet->buddy_id, temp);
		add_room_packet_to_outgoing(client, client->buffer, o);
	}

	return FALSE;
}

static boolean handle_rs_update_ranking_packet(
	struct room_client_data *client, 
	struct room_packet_header *header)
{
	boolean disconnect= FALSE;
	struct rs_update_ranking_packet *packet= (struct rs_update_ranking_packet *) ((char *) header + sizeof(struct room_packet_header));

	if(client->logged_in)
	{
		struct player_stats game_data;
	
		if(get_user_game_data(packet->user_id, client->game_type, &game_data, NULL))
		{
			long score;

			if(packet->stats.caste==NONE)
			{
				boolean set= TRUE;
			
				if(set_user_as_bungie_admin(packet->user_id, set))
				{
					struct room_client_data *room_client;
					
					room_client= (struct room_client_data *) user_parameters.clients;
					while(room_client)
					{
						if(room_client->type==_room_client_type && ((struct room_client_data *) room_client)->logged_in==TRUE)
						{	
							send_user_ranking_packet((struct room_client_data *) room_client, packet->user_id);
						}
						room_client= room_client->next;
					}
				}
			} else {
				short caste;

				add_player_stats(&game_data, &packet->stats, &game_data);
				score= convert_player_stats_to_score(&game_data);
				caste= game_data.caste;
				game_data.updates_since_last_game_played= 0;
				set_user_game_data(packet->user_id, client->game_type, &game_data);


				send_user_ranking_packet(client, packet->user_id);
			}
		} 
		else 
		{
		}
	} 
	else 
	{
		disconnect= TRUE;
	}
	
	return FALSE;
}

static boolean handle_rs_update_room_data_packet(
	struct room_client_data *client, 
	struct room_packet_header *header)
{
	boolean disconnect= FALSE;
	struct rs_update_room_data_packet *packet= (struct rs_update_room_data_packet *) ((char *) header + sizeof(struct room_packet_header));

	if(client->logged_in)
	{
		client->player_count= packet->player_count;
		client->game_count= packet->game_count;
	} else {
		disconnect= TRUE;
	}

	if (send_room_list_to_rooms(FALSE) == FALSE)
	{
	}
	
	return FALSE;
}

static boolean handle_rs_request_ranking_packet(
	struct room_client_data *client, 
	struct room_packet_header *header)
{
	boolean disconnect= FALSE;
	struct rs_request_ranking_packet *packet= (struct rs_request_ranking_packet *) ((char *) header + sizeof(struct room_packet_header));

	if(client->logged_in)
	{
		send_user_ranking_packet(client, packet->player_id);
	} else {
		disconnect= TRUE;
	}
	
	return FALSE;
}

static boolean handle_rs_status_packet(
	struct room_client_data *client,  
	struct room_packet_header *header)
{
	struct rs_status_packet *packet= (struct rs_status_packet *) ((char *) header + sizeof(struct room_packet_header));
	// ALAN Begin: unused variables
//	long *players_in_room= (long *) ((char *) header+sizeof(struct room_packet_header)+sizeof(struct rs_status_packet));
//	short index;
	// ALAN End
	
	printf("Room %d (0x%lx:%d) Players: A: %d IG: %d Gsts: %d Games: A: %d IP: %d\n",
		client->room_identifier, client->host, client->room_port,
		packet->players_available_count, packet->players_in_game_count, packet->guest_player_count,
		packet->games_available_count, packet->games_in_progress_count);

	client->player_count= packet->players_in_game_count+packet->players_available_count+packet->guest_player_count;
	client->game_count= packet->games_available_count+packet->games_in_progress_count;

	if (send_room_list_to_rooms(FALSE) == FALSE)
	{
	}
	
	return FALSE;
}

// MAXIMUM SIZE OF PLAYER DATA (128) + PLAYER_AUX_DATA (16) + ROOM_ID (4) = 148
// 148 * 100 = 14800, + SIZEOF HEADER (8) = 14808
#define	RESPONSE_BUFFER_SIZE	32768
static boolean handle_rs_player_query_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	char * p;
	struct rs_player_query_packet * packet;
	struct user_query query;
	struct user_query_response * qr;

	short type;
	char buffer[RESPONSE_BUFFER_SIZE];
	short length;

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_player_query_packet *)p;
	p += sizeof(struct rs_player_query_packet);
	strncpy(query.string, p, MAXIMUM_LOGIN_LENGTH + 1);
	query.string[MAXIMUM_LOGIN_LENGTH] = '\0';
	memcpy(&query.buddy_ids[0], &packet->buddy_ids[0], sizeof(unsigned long) * MAXIMUM_BUDDIES);
	query.order = packet->order;

	if (query.string[0])
	{
		type = _player_search_query;
		if ((query.buddy_ids[0]) || (query.order))
		{
			return FALSE;
		}
	}
	else if (query.order)
	{
		type = _order_query;
		if (query.string[0] || (query.buddy_ids[0]))
		{
			return FALSE;
		}
	}
	else 
	{
		type = _buddy_query;
		if (query.string[0] || (query.order))
		{
			return FALSE;
		}
	}

	query_user_database(&query, &qr);
	length = build_rs_player_query_response_packet(buffer, type, packet->player_id, qr);

	add_room_packet_to_outgoing(client, buffer, length);

	return FALSE;
}

static boolean handle_rs_player_enter_room_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	struct rs_player_enter_room_packet * packet;
	char * p;
	struct bungie_net_player_datum player;
	short order_member_count;
	struct order_member order_members[STEFANS_MAXIMUM_ORDER_MEMBERS];
	short length;
	short index;
	
	memset(order_members, 0, STEFANS_MAXIMUM_ORDER_MEMBERS * sizeof(struct order_member));
	
	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_player_enter_room_packet *)p;
	if (get_player_information(NULL, packet->player_id, &player))
	{
		short buddy_index;
		struct buddy_entry buddies[MAXIMUM_BUDDIES];
		void * key;

		player.room_id = packet->room_id;

		update_player_information(NULL, packet->player_id, TRUE, &player);

		memcpy(buddies, player.buddies, sizeof(buddies));

		for (buddy_index = 0; buddy_index < MAXIMUM_BUDDIES; buddy_index++)
		{
			if (buddies[buddy_index].player_id)
			{
				if (!is_player_online(buddies[buddy_index].player_id))
				{
					buddies[buddy_index].active = OFFLINE;
				}
			}
			else
			{
				break;
			}
		}

		order_member_count = 0;
		order_members[order_member_count].player_id = get_first_player_in_order(player.order_index, &key);
		while(key && order_member_count<STEFANS_MAXIMUM_ORDER_MEMBERS)
		{
			order_members[order_member_count].online = 
				is_player_online(order_members[order_member_count].player_id);
			order_member_count++;
			order_members[order_member_count].player_id = get_next_player_in_order(&key);
		}

		for (index = 0; index < order_member_count; index++)
		{
			if (order_members[index].online)
			{
				struct room_client_data * temp_client;
				struct bungie_net_online_player_data o_player;

				if (get_online_player_information(order_members[index].player_id, &o_player))
				{
					temp_client = find_room_client_from_room_id(o_player.room_id);
					if (temp_client)
					{
						length = build_rs_update_order_status_packet(temp_client->buffer, order_members[index].player_id,
							order_member_count, order_members);
						add_room_packet_to_outgoing(temp_client, temp_client->buffer, length);
					}
				}
			}
		}

		length = build_rs_update_buddy_response_packet(client->buffer, packet->player_id, buddies);
		add_room_packet_to_outgoing(client, client->buffer, length);

		for (buddy_index = 0; buddy_index < MAXIMUM_BUDDIES; buddy_index++)
		{
			if (buddies[buddy_index].player_id)
			{
				get_player_information(NULL, buddies[buddy_index].player_id, &player);
				if (is_player_online(player.player_id))
				{
					short bi;
					boolean send_update = FALSE;

					for (bi = 0; bi < MAXIMUM_BUDDIES; bi++)
					{
						if (player.buddies[bi].player_id == packet->player_id)
						{
							send_update = TRUE;
						}
						else if (player.buddies[bi].player_id)
						{
							if (!is_player_online(player.buddies[bi].player_id))
							{
								player.buddies[bi].active = OFFLINE;
							}
						}
					}

					if (send_update)
					{
						client = find_room_client_from_room_id(player.room_id);
						if (client)
						{
							length = build_rs_update_buddy_response_packet(client->buffer, player.player_id, player.buddies);
							add_room_packet_to_outgoing(client, client->buffer, length);
						}
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	return FALSE;
}

static boolean handle_rs_player_leave_room_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	struct rs_player_leave_room_packet * packet;
	char * p;
	struct bungie_net_player_datum player;
	short length;
	struct order_member order_members[STEFANS_MAXIMUM_ORDER_MEMBERS];
	short index;
	void * key;
	
	memset(order_members, 0, STEFANS_MAXIMUM_ORDER_MEMBERS * sizeof(struct order_member));

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_player_leave_room_packet *)p;
	if (get_player_information(NULL, packet->player_id, &player))
	{
		if (player.room_id == packet->room_id)
		{
			short buddy_index;
			struct buddy_entry buddies[MAXIMUM_BUDDIES];

			update_player_information(NULL, packet->player_id, FALSE, &player);

			if (player.order_index)
			{
				short order_member_count = 0;
				order_members[order_member_count].player_id = get_first_player_in_order(player.order_index, &key);
				while(key && order_member_count<STEFANS_MAXIMUM_ORDER_MEMBERS)
				{
					order_members[order_member_count].online = 
						is_player_online(order_members[order_member_count].player_id);
					order_member_count++;
					order_members[order_member_count].player_id = get_next_player_in_order(&key);
				}

				for (index = 0; index < order_member_count; index++)
				{
					if (order_members[index].online)
					{
						struct room_client_data * temp_client;
						struct bungie_net_online_player_data o_player;

	
						if (get_online_player_information(order_members[index].player_id, &o_player))
						{
							temp_client = find_room_client_from_room_id(o_player.room_id);
							if (temp_client)
							{
								length = build_rs_update_order_status_packet(temp_client->buffer, order_members[index].player_id,
									order_member_count, order_members);
								add_room_packet_to_outgoing(temp_client, temp_client->buffer, length);
							}
						}
					}
				}
			}

			memcpy(buddies, player.buddies, sizeof(buddies));
			for (buddy_index = 0; buddy_index < MAXIMUM_BUDDIES; buddy_index++)
			{
				if (buddies[buddy_index].player_id)
				{
					get_player_information(NULL, buddies[buddy_index].player_id, &player);
					if (is_player_online(player.player_id))
					{
						short bi;
						boolean send_update = FALSE;

						for (bi = 0; bi < MAXIMUM_BUDDIES; bi++)
						{
							if (player.buddies[bi].player_id == packet->player_id)
							{
								send_update = TRUE;
								player.buddies[bi].active = OFFLINE;
							}
							else if (player.buddies[bi].player_id)
							{
								if (!is_player_online(player.buddies[bi].player_id))
								{
									player.buddies[bi].active = OFFLINE;
								}
							}
						}

						if (send_update)
						{
							client = find_room_client_from_room_id(player.room_id);
							if (client)
							{
								length = build_rs_update_buddy_response_packet(client->buffer, player.player_id, player.buddies);
								add_room_packet_to_outgoing(client, client->buffer, length);
							}
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	
	return FALSE;
}

static boolean handle_rs_update_player_information_packet(
	struct room_client_data * client,
	struct room_packet_header * header)
{
	struct rs_update_player_information_packet * packet;
	struct bungie_net_player_datum player;
	short data_length;
	char * p;

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_update_player_information_packet *)p;
	p += sizeof(struct rs_update_player_information_packet);

	if (get_player_information(NULL, packet->player_id, &player))
	{
		data_length = header->length - (sizeof(struct rs_update_player_information_packet) + 
			sizeof(struct room_packet_header));
		memcpy(player.description, p, data_length);
		update_player_information(NULL, packet->player_id, TRUE, &player);
	}

	return FALSE;
}

static boolean handle_rs_player_info_request_packet(
	struct room_client_data * client, 
	struct room_packet_header * header)
{
	struct rs_player_info_request_packet * packet;
	struct bungie_net_player_stats stats;
	char * p, * q;
	short length;

	struct bungie_net_player_datum player;
	struct bungie_net_order_datum order;

	p = (char *)header;
	p += sizeof(struct room_packet_header);

	packet = (struct rs_player_info_request_packet *)p;
	if (packet->requested_player_id)
	{
		if (get_player_information(NULL, packet->requested_player_id, &player))
		{
			stats.administrator_flag = player.administrator_flag;
			stats.bungie_employee_flag = player.special_flags & _bungie_employee_flag;
			stats.order_index = player.order_index;
			stats.icon_index = player.icon_index;
			stats.primary_color = player.primary_color;
			stats.secondary_color = player.secondary_color;
			stats.unranked_score_datum = player.unranked_score;
			stats.ranked_score_datum = player.ranked_score;
			strcpy(stats.login, player.login);
			strcpy(stats.name, player.name);
			p = player.description;
			q = stats.description;

			length = strlen(p) + 1;
			strcpy(q, p);

			p += length;
			q += length;
			length = strlen(p) + 1;
			strcpy(q, p);

			p += length;
			q += length;
			length = strlen(p) + 1;
			strcpy(q, p);

			p += length;
			q += length;
			length = strlen(p) + 1;
			strcpy(q, p);

			memcpy(&stats.ranked_score_datum_by_game_type[0], &player.ranked_scores_by_game_type[0], 
				sizeof(struct bungie_net_player_score_datum) * MAXIMUM_NUMBER_OF_GAME_TYPES);

			if (player.order_index)
			{
				if (get_order_information(NULL, player.order_index, &order))
				{
					stats.order_unranked_score_datum = order.unranked_score;
					stats.order_ranked_score_datum = order.ranked_score;
					memcpy(&stats.order_ranked_score_datum_by_game_type[0], &order.ranked_scores_by_game_type[0], 
						sizeof(struct bungie_net_player_score_datum) * MAXIMUM_NUMBER_OF_GAME_TYPES);
					strcpy(stats.order_name, order.name);
				}
				else
				{
					stats.order_index = 0;
				}
			}

			length = build_rs_player_info_reply_packet(client->buffer, packet->player_id, &stats);
			add_room_packet_to_outgoing(client, client->buffer, length);
		}
	}

	return FALSE;
}

static void send_user_ranking_packet(
	struct room_client_data *client,
	long player_id)
{
	struct player_stats stats;
	short user_flags;

	if(get_user_game_data(player_id, client->game_type, &stats, &user_flags))
	{
		short length;
	
		length= build_rs_client_ranking_packet(client->buffer, player_id, 0, &stats);

		add_room_packet_to_outgoing(client, client->buffer, length);
	}
	
	return;
}

static boolean send_room_list_to_rooms(
	boolean force_send)
{
	static long last_update_ticks= 0l;
	boolean success = TRUE;
	
	if(force_send || machine_tick_count()-last_update_ticks>TICKS_BEFORE_UPDATING_ROOM_LIST)
	{
		struct room_client_data *client= (struct room_client_data *) user_parameters.clients;
		struct room_client_data *room= (struct room_client_data *) user_parameters.clients;
		char room_buffer[8*KILO];
		short length;

		length= start_building_rs_room_packet(room_buffer);
		while(room)
		{
			if(room->type==_room_client_type && room->logged_in==TRUE)
			{
				struct player_room_list_data info;
				
				info.info.room_id= room->room_identifier;
				info.info.player_count= room->player_count;
				info.info.host= room->host;
				info.info.port= room->room_port;
				info.info.game_count= room->game_count;
				info.country_code= room->country_code;
				info.minimum_caste= room->minimum_caste;
				info.maximum_caste= room->maximum_caste;
				info.tournament_room = room->tournament_room;

				if (room->tournament_room)
				{
					info.info.room_type= _tournament_room;
				}
				else if (room->ranked_room) 
				{
					info.info.room_type= _ranked_room;
				}
				else
				{
					info.info.room_type= _unranked_room;
				}
				
				length= add_room_to_rs_room_packet(room_buffer, &info);
			}
			room= (struct room_client_data *) room->next;
		}
		if(length<0 || length>=sizeof(room_buffer))
			return FALSE;

		while(client)
		{
			if(client->type==_room_client_type && client->logged_in)
			{
				if (add_room_packet_to_outgoing(client, room_buffer, length) == FALSE)
				{
				success = FALSE;
				}
			}
			client= (struct room_client_data *) client->next;
		}
		
		last_update_ticks= machine_tick_count();
	}
	
	return success;
}

static short find_next_available_room(
	short identifier_hint,
	short *game_type, 
	short *ranked_room,
	short *tournament_room,
	short *country_code,
	short *minimum_caste,
	short *maximum_caste)

{
	struct room_data *best_room= (struct room_data *) NULL;
	short room_id= NONE;

	{	
		struct room_data *room;

		// find the next available room or match the identifier if not NONE
		for (room= user_parameters.rooms; room; room= room->next)
		{
			if (!room->used)
			{
				if (!best_room)
				{
					best_room= room;
					if (identifier_hint==NONE) break;
				}
				
				if (identifier_hint!=NONE && identifier_hint==room->room_identifier)
				{
					best_room= room;
					break;
				}
			}
		}
	}

	if (best_room)
	{			
		best_room->used= TRUE;
		*game_type= best_room->game_type;
		*ranked_room= best_room->ranked_room;
		*country_code= best_room->country_code;
		*minimum_caste= best_room->minimum_caste;
		*maximum_caste= best_room->maximum_caste;
		*tournament_room = best_room->tournament_room;

		room_id= best_room->room_identifier;
	}
			
	return room_id;
}

static void mark_room_as_available(
	short game_type,
	short room_identifier)
{
	struct room_data *room;

	room= user_parameters.rooms;
	while(room)
	{
		if(room->used && room->game_type==game_type && room->room_identifier==room_identifier)
		{
			printf("Room logged off.  Making it available again...\n");
			room->used= FALSE;
			break;
		}
		room= room->next;
	}

	return;
}

/* --------------- utility routines */
static struct client_data *attach_client_to_list(
	struct client_data *clients,
	struct client_data *client)
{
	client->next= NULL;
	if(!clients)
	{
		clients= client;
	} else {
		struct client_data *previous;
		
		previous= clients;
		while(previous->next != NULL) previous= previous->next;
		previous->next= client;
	}
	
	return clients;
}

static struct client_data *add_client(
	struct client_data *clients, 
	int socket, 
	long host, 
	short port,
	short type)
{
	struct client_data *client;
	long length= 0;
	char *type_name= 0;
	
	switch(type)
	{
		case _player_client_type: length= sizeof(struct player_client_data); type_name= "player incoming"; break;
		case _room_client_type: length= sizeof(struct room_client_data); type_name= "room"; break;
		case _web_client_type: length= sizeof(struct web_client_data); type_name= "new user"; break;
		default: halt(); break;
	}

	
	client= (struct client_data *) malloc(length);
	if(client)
	{
		memset(client, 0, length);
		client->socket= socket;
		client->host= host;
		client->port= port;
		client->type= type;
		client->state= _awaiting_login_packet_state;
		if(allocate_circular_queue(type_name, &client->incoming, INCOMING_QUEUE_SIZE))
		{
			if(allocate_circular_queue("blahblahblah", &client->outgoing, OUTGOING_QUEUE_SIZE))
			{
				clients= attach_client_to_list(clients, client);
			} else {
				free_circular_queue(&client->incoming);
				free(client);
			}
		} else {
			free(client);
		}
	} else {
	}
	
	return clients;
}

static struct room_client_data * find_room_client_from_room_id(
	short room_id)
{
	struct client_data * client = user_parameters.clients;

	while (client)
	{
		if (client->type == _room_client_type)
		{
			if (((struct room_client_data *)(client))->room_identifier == room_id)
			{
				break;
			}
		}

		client = client->next;
	}

	return (struct room_client_data *)client;
}

static struct client_data *find_client_from_socket(
	struct client_data *clients,
	int socket)
{
	struct client_data *client= clients;
	
	while(client && client->socket != socket) client= client->next;
	
	return client;
}

static struct client_data *delete_client(
	struct client_data *clients,
	struct client_data *dead)
{
	switch(dead->type)
	{
		case _player_client_type: 
		case _web_client_type: 
			break;
			
		case _room_client_type: 
			{
				struct room_client_data *c= (struct room_client_data *) dead;
				mark_room_as_available(c->game_type, c->room_identifier);
			}
			break;
			
		default: 
			halt(); 
			break;
			
	}

	if(clients==dead)
	{
		clients= dead->next;
	} else {
		struct client_data * previous= clients;
		
		while (previous && previous->next != dead)
			previous= previous->next;
		if(previous)
			previous->next = dead->next;
	}
	free_circular_queue(&dead->incoming);
	free_circular_queue(&dead->outgoing);
	free(dead);
	
	return clients;
}

static int create_listening_socket(
	short port)
{
	int new_socket;
	int server_socket = -1;

	new_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (new_socket != -1) 
	{
		struct sockaddr_in server_address;
		int option_value;

		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		server_address.sin_port = htons(port); 
	    option_value = TRUE;
		if(setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value))!=0)
		{
		}

		if(bind(new_socket, (struct sockaddr *) &server_address, sizeof(server_address))!=-1) 
		{
			if(listen(new_socket, MAXIMUM_OUTSTANDING_REQUESTS)==0)
			{
				server_socket= new_socket;
			} else {
			}
		} else {
		}
	} else {
	}
	
	return server_socket;
}

static void login_next_queued_room(
	struct client_data *clients)
{
	struct client_data *client;
	boolean logged_in_replacement_room= FALSE;
	
	client= clients;
	while(client && !logged_in_replacement_room)
	{
		if(client->type==_room_client_type)
		{
			struct room_client_data *room_client= (struct room_client_data *) client;
			
			if(!room_client->logged_in)
			{
				logged_in_replacement_room= try_to_login_client_room(room_client, NONE);
				if(!logged_in_replacement_room)
				{
				}
			}
		}
		client= client->next;
	}

	if (send_room_list_to_rooms(TRUE) == FALSE)
	{
	}
	
	return;
}

static boolean try_to_login_client_room
	(
	struct room_client_data *client,
	short identifier_hint
	)
{
	short ranked_room, country_code, minimum_caste, maximum_caste, tournament_room;
	char url_for_update[ROOM_MAXIMUM_UPDATE_URL_SIZE];
	boolean logged_in_room= FALSE;
	char * motd;

	*url_for_update= '\0';
	
	client->room_identifier= find_next_available_room(identifier_hint, &client->game_type, &ranked_room, &tournament_room, &country_code,&minimum_caste, &maximum_caste);
	if (client->room_identifier != NONE)
	{
		short length;
		client->player_count = 0;

		motd = load_motd();
	 
		length= build_rs_login_successful_packet(client->buffer, client->room_identifier, client->game_type, 0, 0, ranked_room, tournament_room, &caste_breakpoints, url_for_update, motd);
		add_room_packet_to_outgoing(client, client->buffer, length);
		printf("logged in room at 0x%lx:%d \n Game ID: %d\n Room ID: %d\n Country Code: %d\n Min Caste: %d\n Max Caste: %d\n Update url: %s\n MOTD: %s\n", 
			client->host, client->room_port,
			client->game_type, client->room_identifier,
			country_code,
			minimum_caste,
			maximum_caste,
			url_for_update,
			motd);

		client->country_code= country_code;
		client->minimum_caste= minimum_caste;
		client->maximum_caste= maximum_caste;
		client->tournament_room = tournament_room;
		client->ranked_room = ranked_room;
		client->logged_in= TRUE;
		logged_in_room= TRUE;
	}
	else
	{
		printf("Not enough room identifiers.  Queuing the room to await a room death....\n");
	}
	
	return logged_in_room;
}

static char * load_motd(void)
{
	static boolean motd_initialized = FALSE;
	char filename[256];

	sprintf(filename, "%s/%s", get_metaserver_root_dir(), get_motd_file_name());

	if (!motd_initialized)
	{
		FILE * fp;

		if (access(filename, F_OK) == 0)
		{
			user_parameters.motd[0] = '\0';
			fp = fopen(filename, "r");
			if (fp != NULL)
			{
				fgets(user_parameters.motd, sizeof(user_parameters.motd), fp);
				fclose(fp);
			}
			motd_initialized = TRUE;
		}
	}

	return user_parameters.motd;
}

static void save_motd(void)
{
	FILE * fp;
	char filename[256];

	sprintf(filename, "%s/%s", get_metaserver_root_dir(), get_motd_file_name());
		
	fp= fopen(filename, "w+");
	if(fp)
	{
		fprintf(fp, user_parameters.motd);
		fflush(fp);
		fclose(fp);
	}
	
	return;
}

static short build_game_specific_login_packet(
	struct player_client_data *client)
{
	short length= 0;

	switch(client->game_type)
	{
		// _MYTHDEV Begin
	//	case 0:
		case 1: // Myth1
		case 2: // Myth2
		case 3: // Myth3
		// _MYTHDEV End
			length= build_myth_specific_login_packet(client);
			break;
			
		default:
			break;
	}
	
	return length;
}

static short build_myth_specific_login_packet(
	struct player_client_data *client)
{
	short length= 0;

	// _MYTHDEV Begin:  Minimal support for new client login scheme
//	if(client->game_type!=0)
	if( client->game_type!=1 && client->game_type!=2 && client->game_type!=3 )
	// _MYTHDEV End
		return 0;

	if(client->user_id) 
	{
		char myth_player_data[128];
		short myth_player_data_size;
		long ranking= 0;
		short caste= NONE;
		struct player_stats stats;
		struct metaserver_player_aux_data player_aux_data;

		if(client->reset_player_data)
		{
			set_myth_user_data(client->user_id, client->player_data, client->player_data_size);
		}
		
		if(get_user_game_data(client->user_id, client->game_type, &stats, NULL))
		{
			ranking= convert_player_stats_to_score(&stats);
		} else {
		}
		
		{
			struct bungie_net_player_datum player;

			get_player_information(NULL, client->user_id, &player);

			get_myth_user_data(client->user_id, myth_player_data, &myth_player_data_size);
			
			player_aux_data.verb = 0;
			player_aux_data.flags = 0;
			player_aux_data.ranking = ranking;
			player_aux_data.player_id = client->user_id;
			player_aux_data.caste = caste;
			if(myth_player_data_size > sizeof(myth_player_data))
				return 0;
			player_aux_data.player_data_length = myth_player_data_size;
		}

		length= build_set_player_data_from_metaserver_packet(client->buffer, &player_aux_data, myth_player_data, myth_player_data_size);
	}
	
	return length;
}

static void send_room_packet_to_all_connected_rooms(
	char *buffer,
	short length)
{
	struct client_data *client;
	
	client= user_parameters.clients;
	while(client)
	{
		if(client->type==_room_client_type)
		{	
			struct room_client_data *room_client= (struct room_client_data *) client;
		
			if(room_client->logged_in)
			{
				add_room_packet_to_outgoing(room_client, buffer, length);
			}
		}
		client= client->next;
	}
	
	return;
}

/* ALAN Begin: this function defined but not used
static boolean send_packet(
	int socket, 
	char *buffer, 
	short length)
{
	short sent_length;
	boolean disconnect= FALSE;
	
	byteswap_packet(buffer, TRUE);
	sent_length= send(socket, buffer, length, 0);
	if(sent_length==-1)
	{
		perror("send_packet errored out");
		disconnect= TRUE;
	} else
	{
		if(sent_length!=length)
			return TRUE;
	}
	byteswap_packet(buffer, FALSE);

	return disconnect;
}
// ALAN End  */


/* _MYTHDEV Begin:  Original function (for reference)
static boolean add_packet_to_outgoing(
	struct client_data *client,
	char *buffer,
	short length)
{
	boolean success;

	if (client->type != _web_client_type) byteswap_packet(buffer, TRUE);
	success= copy_data_into_circular_queue(buffer, length, &client->outgoing);
	if (client->type != _web_client_type) byteswap_packet(buffer, FALSE);
	
	return success;
}
 _MYTHDEV End 
*/

static boolean add_packet_to_outgoing(
	struct client_data *client,
	char *buffer,
	short length)
{
	boolean success;

    // _MYTHDEV Begin
    // This method of backing up the buffer is arguable faster than
    // encrypting and byte swapping and then decrypting and byte swapping again.
    char* backup_buffer = malloc( length );
    int original_length = length;
    memcpy( backup_buffer, buffer, length ); 

    if ( client->type == _web_client_type ) {
    } else if ( client->type == _player_client_type ) {
        if ( ((struct player_client_data*)client)->valid_session_key ) {
            length = encryptData( client, buffer, length );
        } else {                                                                       
		    byteswap_packet(buffer, TRUE);
        }     
    } else {
        byteswap_packet(buffer, TRUE);
    }
    // _MYTHDEV End

    success= copy_data_into_circular_queue(buffer, length, &client->outgoing);

    // _MYTHDEV Begin:  Undo changes and leave buffer untouched    
    memcpy( buffer, backup_buffer, original_length ); 
    free ( backup_buffer );
    // _MYTHDEV End

	return success;
}

static boolean add_room_packet_to_outgoing(
	struct room_client_data *client,
	char *buffer,
	short length)
{
	boolean success;

	if(client->type!=_room_client_type)
		return FALSE;
	
	byteswap_room_packet(buffer, TRUE);
	success= copy_data_into_circular_queue(buffer, length, &client->outgoing);
	byteswap_room_packet(buffer, FALSE);
	
	return success;
}

static void build_client_writefds(
	fd_set *write_fds)
{
	struct client_data *client= user_parameters.clients;

	FD_ZERO(write_fds);
	while(client)
	{
		if(circular_buffer_size(&client->outgoing))
		{
			FD_SET(client->socket, write_fds);
		}
		client= client->next;
	}

	return;
}

static boolean send_outgoing_data(
	struct client_data *client)
{
	boolean done= FALSE;
	boolean error= FALSE;
	
	do {
		long size_to_write;
		
		size_to_write= circular_buffer_linear_write_size(&client->outgoing);
		if(size_to_write)
		{
			long length_sent;
		
			length_sent= send(client->socket, client->outgoing.buffer+client->outgoing.read_index, size_to_write, 0);
			if(length_sent == -1)
			{
				error= TRUE;
				done= TRUE;
			} else {
				if(length_sent != size_to_write)
				{
					done= TRUE;
				} else {
				}
				client->outgoing.read_index+= length_sent;
				if(client->outgoing.read_index>=client->outgoing.size) client->outgoing.read_index= 0;
			}
		} else {
			done= TRUE;
		}
	} while(!done);
	
	return error;
}


// ALAN Begin: modified so we can build BN2_DEMOVERSION warning free
#ifdef BN2_FULLVERSION
static void get_valid_client_salt(
	unsigned long user_id,
	char *salt,
	short authentication_type)
{
	struct bungie_net_player_datum user;

	if(get_player_information(NULL, user_id, &user))
	{
		char encrypted_password[MAXIMUM_ENCRYPTED_PASSWORD_SIZE];

		get_random_salt(salt);
		encrypt_password(user.password, salt, encrypted_password, authentication_type);
	}
	
	return;
}
#endif
// ALAN End

static boolean login_user(
	long user_id, 
	char *password,
	unsigned char *salt,
	short authentication_type,
	boolean *first_time)
{
	boolean success= FALSE;
	struct bungie_net_player_datum user;

	if(get_player_information(NULL, user_id, &user))
	{
		char encrypted_password[MAXIMUM_ENCRYPTED_PASSWORD_SIZE];
	
		encrypt_password(user.password, salt, encrypted_password, authentication_type);
		// ALAN Begin
		// Bungie removed password encryption, you'll have to write your own
	//	if(memcmp(encrypted_password, password, MAXIMUM_ENCRYPTED_PASSWORD_SIZE)==0)
		if( 1 )
		// ALAN End
		{
			*first_time = (user.last_login_time == 0L);
			user.last_login_time = get_current_time();
			success = update_player_information(NULL, user_id, FALSE, &user);
		}
	}
	
	return success;
}

static boolean handle_change_motd(char * buffer)
{
	short length;
	char outgoing_buffer[1024];

	strcpy(user_parameters.motd, buffer);
	save_motd();
	printf("MOTD set to: %s\n", buffer);

	length = build_rs_motd_changed_packet(outgoing_buffer, user_parameters.motd);
	send_room_packet_to_all_connected_rooms(outgoing_buffer, length);
	
	return FALSE;
}

static boolean reset_player_scores_by_game_type(
	short type)
{
	struct bungie_net_player_datum player;
	boolean success= FALSE;

	if (get_first_player_information(&player))
	{
		success= TRUE;
		do
		{
			memset(&player.ranked_scores_by_game_type[type], 0, sizeof(struct bungie_net_player_score_datum));
			scoring_datum_adjust_total(player.ranked_scores_by_game_type, &player.ranked_score);
			update_player_information(NULL, player.player_id, is_player_online(player.player_id), &player);
		} while (get_next_player_information(&player));
	}

	return success;
}

static boolean reset_order_scores_by_game_type(
	short type)
{
	struct bungie_net_order_datum order;
	boolean success= FALSE;

	if (get_first_order_information(&order))
	{
		success= TRUE;
		do
		{
			memset(&order.ranked_scores_by_game_type[type], 0, sizeof(struct bungie_net_player_score_datum));
			scoring_datum_adjust_total(order.ranked_scores_by_game_type, &order.ranked_score);
			update_order_information(NULL, order.order_id, &order);
		} while (get_next_order_information(&order));
	}

	return success;
}

#include <stdarg.h>
int my_printf(const char *format, ...)
{
	char buff[256];
	va_list ap;
	int ioerr;
	
	va_start(ap, format);
	vsnprintf(buff, 256, format, ap);
	ioerr = fprintf(stdout, buff);
	va_end(ap);
	
	return ioerr;
}

// _MYTHDEV Begin
struct myth_metaserver_player_data {
	char coat_of_arms_bitmap_index;
	char caste_bitmap_index;
	short state;
	struct rgb_color primary_color;
	struct rgb_color secondary_color;
	char filler[18];
};
// _MYTHDEV End

// _MYTHDEV Begin
#ifdef BN2_FULLVERSION
static int get_build_version_from_player_data( 
	struct player_client_data *player)
{
	char *p;

	if( !player )
		return 0;

	p = player->player_data;
	p += sizeof(struct myth_metaserver_player_data) - (2*sizeof(struct rgb_color));

	return SWAP2(*(word *)p);
}
#endif
// _MYTHDEV End

// _MYTHDEV Begin
static boolean decryptData( struct packet_header* packet_header, struct player_client_data * player_client_data ) 
{
    if ( player_client_data->valid_session_key ) {
      char* pData = ((char*)packet_header + sizeof(struct packet_header));  
        #ifdef little_endian
            int data_size = SWAP4(packet_header->length) - sizeof(struct packet_header);
        #else
            int data_size = packet_header->length - sizeof(struct packet_header);
        #endif

        data_size = decodeData( pData, data_size, player_client_data->session_key );
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
    struct player_client_data* player_client_data = (struct player_client_data*)client;
    int header_size = sizeof(struct packet_header);     
    struct packet_header* packet_header = (struct packet_header*)buffer;
    char*  pData = buffer + header_size;
    int data_size = length - header_size;

    byteswap_packet(buffer, TRUE);

    data_size = encodeData( pData, data_size, player_client_data->session_key ); 

    packet_header->length = data_size + header_size;
    length = packet_header->length;    

    #ifdef little_endian
	    packet_header->length= SWAP4(packet_header->length);
    #endif
    
    return length;
}
// _MYTHDEV End
