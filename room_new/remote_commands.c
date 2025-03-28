/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include "byte_swapping.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "metaserver_common_structs.h"
#include "stats.h"
#include "authentication.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "rank.h"
#include "games.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "server_code.h"
#include "remote_commands.h"
#include "room_globals.h"
#include "environment.h"

#include "game_search_packets.h"


/* ------- structures */

enum {
	_myth_metaserver_chat_packet,
	NUMBER_OF_MYTH_METASERVER_PACKETS
};

struct myth_metaserver_chat_packet {
	short type;
	short length;
	struct rgb_color color;
	word flags;
	word pad;
	unsigned long player_id;
	unsigned long selected_player_id;
};

/* ------- local prototypes */
static boolean try_and_handle_remote(struct client_data *client, char *message, unsigned long selected_player_id);

/* ------- code */
boolean handle_remote_command(
	struct client_data *client, 
	char *message, 
	short length)
{
	boolean handled= FALSE;
	
	if(length>sizeof(struct myth_metaserver_chat_packet))
	{
		struct myth_metaserver_chat_packet *packet= (struct myth_metaserver_chat_packet *) message;
		short type= packet->type;
		unsigned long selected_player_id= packet->selected_player_id;
		
#ifdef little_endian
		type= SWAP2(type);
		selected_player_id= SWAP4(packet->selected_player_id);
#endif

		switch(type)
		{
			case _myth_metaserver_chat_packet:
				{
					char *user= message+sizeof(struct myth_metaserver_chat_packet);
					char *chat= user+strlen(user)+1;
					
					if(chat[0]=='.')
					{
						handled= try_and_handle_remote(client, chat, selected_player_id);
					}
				}
				break;
			
			default:
				break;
		}
	} 
	
	return handled;
}

/* ------- local code */

static void handle_help(struct client_data *client, char *rest, unsigned long selected_player_id);


void change_room_motd(char *motd);
struct {
	char *command;
	char *description;
	boolean admin_only;
	void (*handler)(struct client_data *client, char *rest, unsigned long selected_player_id);
} remote_commands[]= {
	{ ".help", "Prints this text", FALSE, handle_help }
};
#define NUMBER_OF_REMOTE_COMMANDS (sizeof(remote_commands)/sizeof(remote_commands[0]))

static boolean try_and_handle_remote(
	struct client_data *client, 
	char *message, 
	unsigned long selected_player_id)
{
	short index;
	boolean handled= FALSE;
	
	for(index= 0; index<NUMBER_OF_REMOTE_COMMANDS; ++index)
	{
		short length= strlen(remote_commands[index].command);

		if (memcmp(remote_commands[index].command, message, length) == 0)
		{
			if (remote_commands[index].admin_only == TRUE)
			{
				if (is_player_admin(client) == FALSE)
				{
					return FALSE;
				}
			}

			remote_commands[index].handler(client, message+length, selected_player_id);
			handled= TRUE;
			break;
		}
	}
	
	return handled;
}

/* ----------- handlers */
static void handle_help(
	struct client_data *client, 
	char *rest, 
	unsigned long selected_player_id)
{
	if(rest && rest[0] != '\0')
	{
		send_room_message_to_player(client, "I'm sorry, I didn't understand your command. Please try again.");
		return;
	}
	send_room_message_to_player(client, ".help - prints this message");
	
	return;
}
