/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include <string.h>

#include "environment.h"
#include "room_list_file.h"

#define printf		my_printf
int my_printf(const char *format, ...);

static char *game_type_names[]= {
	"MYTH"
};
#define NUMBER_OF_GAME_TYPES (sizeof(game_type_names)/sizeof(game_type_names[0]))

/* ------ local prototypes */
static struct room_data *add_room(struct room_data *rooms,
	short game_type, short room_id, short ranked_room,
	short country_code, short minimum_caste, short maximum_caste, short tournament_room);

/* ------ code */
struct room_data *load_room_list(void)
{
	char filename[256] = {0};
	FILE *fp = NULL;
	struct room_data * rooms= NULL;

	strcpy(filename, get_rooms_list_file());
	fp= fopen(filename, "r");
	if(fp)
	{
		char game_name[60];
		int room_identifier;
		int ranked;
		int country_code;
		int minimum_caste;
		int maximum_caste;
		int tournament_room;
		
		while (fscanf(fp, "%s %d %d %d %d %d %d \n", game_name, &room_identifier, &ranked,
			&country_code, &minimum_caste, &maximum_caste, &tournament_room) != EOF)
		{
			short game_type= game_name_to_type(game_name);
			
			if(game_type != NONE)
			{
				rooms= add_room(rooms, game_type, room_identifier, ranked, 
					country_code, minimum_caste, maximum_caste, tournament_room);
			} else {
				printf("Unrecognized game name in room list file '%s'\n", game_name);
			}
		}
		fclose(fp);
	}
	else
	{
		printf("No rooms list file found! The userd will not be able to load any rooms!\n");
		printf("bailing out...\n");
		exit(1);
	}
	return rooms;
}	

short game_name_to_type(
	char *name)
{
	short game_type, actual_game_type= NONE;
		
	for(game_type= 0; game_type<NUMBER_OF_GAME_TYPES; ++game_type)
	{
		if(strcasecmp(name, game_type_names[game_type])==0)
		{
			actual_game_type= game_type;
			break;
		}
	}
	
	return actual_game_type;
}

char *get_game_name_from_type(
	short type)
{
	if(type<0 && type>=NUMBER_OF_GAME_TYPES)
		return NULL;
	return game_type_names[type];
}

boolean save_room_list(
	struct room_data *rooms,
	char *filename)
{
	boolean success= FALSE;
	FILE *fp;

	fp= fopen(filename, "w+");
	if(fp) 
	{	
		struct room_data *room= rooms;	

		while(room) 
		{
			fprintf(fp, "%s %d %d %d %d %d %d\n",
			get_game_name_from_type(room->game_type),
			room->room_identifier,
			room->ranked_room,
			room->country_code,
			room->minimum_caste,
			room->maximum_caste,
			room->tournament_room);
			room= room->next;
		}
		fclose(fp);
		success= TRUE;
	}

	return success;
}

void list_room_templates(
	struct room_data *room)
{
	printf("Game\tRoomID\tRanked\tCountry\tMin Caste\tMax Caste\tTournament Room#\n");
	while(room) 
	{
		printf("%s %d %d %d %d %d %d\n",
			get_game_name_from_type(room->game_type),
			room->room_identifier,
			room->ranked_room,
			room->country_code,
			room->minimum_caste,
			room->maximum_caste,
			room->tournament_room);
		room= room->next;
	}

	return;
}

struct room_data *delete_room_template(
	struct room_data *rooms,
	short game_type,
	short room_identifier)
{
	if(game_type != NONE)
	{
		struct room_data *room, *previous;
		
		previous= NULL;
		room= rooms;
		while(room && room->game_type!=game_type && room->room_identifier != room_identifier)
		{
			previous= room;
			room= room->next;
		}
		
		if(room)
		{
			if(previous)
			{
				previous->next= room->next;
			} else {
				rooms= room->next;
			}
			free(room);
			printf("Room deleted!\n");
		} else {
			printf("Room not found!\n");
		}
	} else {
		printf("Unrecognized game type!\n");
	}
	
	return rooms;
}

struct room_data *add_or_update_room(
	struct room_data *rooms,
	short game_type,
	short room_identifier,
	short ranked_room,
	short country_code,
	short minimum_caste,
	short maximum_caste,
	short tournament_room)
{
	if(game_type != NONE) 
	{
		struct room_data *room;

		room= rooms;
		while(room) 
		{
			if(room->game_type==game_type && 
				room->room_identifier==room_identifier) 
			{
				room->ranked_room= ranked_room;
				room->country_code= country_code;
				room->minimum_caste= minimum_caste;
				room->maximum_caste= maximum_caste;
				room->tournament_room = tournament_room;
				printf("Room updated!\n");
				break;
			}

			room= room->next;
		}

		if(!room) 
		{
			rooms= add_room(rooms, game_type, room_identifier, ranked_room,
				country_code, minimum_caste, maximum_caste, tournament_room);
		}
	} else {
		printf("Unrecognized game type! Valid types are:\n");
	}

	return rooms;
}

/* --------- local code */
static struct room_data *add_room(
	struct room_data *rooms,
	short game_type,
	short room_id,
	short ranked_room,
	short country_code,
	short minimum_caste,
	short maximum_caste,
	short tournament_room)
{
	struct room_data *room= (struct room_data *) malloc(sizeof(struct room_data));

	if(room) 
	{
		memset(room, 0, sizeof(struct room_data));
		room->game_type= game_type;
		room->room_identifier= room_id;
		room->ranked_room= ranked_room;
		room->country_code= country_code;
		room->minimum_caste= minimum_caste;
		room->maximum_caste= maximum_caste;
		room->tournament_room = tournament_room;
		room->used= FALSE;

		if(!rooms) 
		{
			rooms= room;
		} else {
			struct room_data *previous= rooms;

			while(previous->next != NULL) previous= previous->next;
			previous->next= room;
		}
	} else {
		printf("Out of memory adding room!\n");
	}

	return rooms;
}
