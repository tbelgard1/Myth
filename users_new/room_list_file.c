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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "../common/cJSON.h"
#include "environment.h"
#include "room_list_file.h"

#define printf	my_printf
int my_printf(const char *format, ...);

/* _MYTHDEV Begin:  Original (for reference)
static char *game_type_names[]= {
	"MYTH"
};
 _MYTHDEV End
*/

// _MYTHDEV Begin
static char *game_type_names[]= {
	"MYTH", "MYTH1", "MYTH2", "MYTH3"
};
// _MYTHDEV End

#define NUMBER_OF_GAME_TYPES (sizeof(game_type_names)/sizeof(game_type_names[0]))

struct room_data *load_room_list(void)
{
    char room_json[MAX_ROOM_JSON_SIZE];
    struct room_data *rooms = NULL;
    if (load_rooms_from_json_file(room_json, sizeof(room_json), "../rooms.json")) {
        if (!parse_rooms_json(room_json, &rooms)) {
            fprintf(stderr, "[ERROR] Failed to parse JSON from JSON loader\n");
            return NULL;
        } else {
            printf("[DEBUG] Successfully parsed rooms from JSON\n");
        }
    } else {
        fprintf(stderr, "[ERROR] Could not load room list from JSON file\n");
        return NULL;
    }
    return rooms;
}


int load_rooms_from_python(char *json_buffer, size_t buffer_size) {
    FILE *fp = popen("python3 /src/spaghetti/load_rooms.py", "r");
    if (fp == NULL) {
        fprintf(stderr, "[ERROR] Failed to run load_rooms.py\n");
        return 0;
    }

    size_t total_read = fread(json_buffer, 1, buffer_size - 1, fp);
    json_buffer[total_read] = '\0'; // null-terminate
    int exit_code = pclose(fp);

    if (exit_code != 0) {
        fprintf(stderr, "[ERROR] load_rooms.py exited with code %d\n", exit_code);
        return 0;
    }

    return 1;
}

// NOTE: Ensure you have cJSON.c/h from https://github.com/DaveGamble/cJSON in common/ and add ../common/cJSON.o to your Makefile
// NOTE: Ensure rooms.json is generated at build/startup: RUN python3 /src/spaghetti/load_rooms.py > /src/spaghetti/rooms.json
#include "room_list_file.h"

int parse_rooms_json(const char *json_str, struct room_data **rooms_out) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) return 0;
    int count = cJSON_GetArraySize(root);
    struct room_data *head = NULL;
    struct room_data *tail = NULL;
    for (int i = 0; i < count; ++i) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (!item) continue;
        cJSON *game_type = cJSON_GetObjectItem(item, "game_type");
        cJSON *ranked_room = cJSON_GetObjectItem(item, "ranked_room");
        cJSON *room_identifier = cJSON_GetObjectItem(item, "id");
        cJSON *country_code = cJSON_GetObjectItem(item, "country_code");
        cJSON *minimum_caste = cJSON_GetObjectItem(item, "minimum_caste");
        cJSON *maximum_caste = cJSON_GetObjectItem(item, "maximum_caste");
        cJSON *tournament_room = cJSON_GetObjectItem(item, "tournament_room");
        if (!game_type || !ranked_room || !room_identifier) continue;
        struct room_data *room = calloc(1, sizeof(struct room_data));
        room->game_type = game_type->valueint;
        room->ranked_room = ranked_room->valueint;
        room->room_identifier = room_identifier->valueint;
        room->country_code = country_code ? country_code->valueint : 0;
        room->minimum_caste = minimum_caste ? minimum_caste->valueint : 0;
        room->maximum_caste = maximum_caste ? maximum_caste->valueint : 0;
        room->tournament_room = tournament_room ? tournament_room->valueint : 0;
        room->used = 0;
        room->next = NULL;
        if (!head) head = tail = room;
        else { tail->next = room; tail = room; }
    }
    cJSON_Delete(root);
    *rooms_out = head;
    return 1;
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
struct room_data *add_room(
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
