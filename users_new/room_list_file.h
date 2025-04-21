/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __ROOM_LIST_FILE_H__
#define __ROOM_LIST_FILE_H__

#include "cseries.h"
#include "../common/cJSON.h"

#define ROOM_JSON_BUFFER_SIZE 8192
#define MAX_ROOM_JSON_SIZE 8192

typedef struct {
    int count;
    struct {
        int room_id;
        char name[128];
        char description[256];
        short game_type;
        short ranked_room;
        short room_identifier;
        short country_code;
        short minimum_caste;
        short maximum_caste;
        short tournament_room;
    } rooms[128];
} room_list_t;

#define MAX_ROOM_NAME_LENGTH 128
#define MAX_ROOM_DESC_LENGTH 256

struct room_data {
    int room_id;
    char name[MAX_ROOM_NAME_LENGTH];
    char description[MAX_ROOM_DESC_LENGTH];
    short game_type;
    short ranked_room;
    short room_identifier;
    short country_code;
    short minimum_caste;
    short maximum_caste;
    short tournament_room;
    short used;
    struct room_data *next;
};

int parse_rooms_json(const char *json_str, struct room_data **rooms_out);
int load_rooms_from_json_file(char *buffer, size_t size, const char *filename);
void free_room_list(room_list_t *list);
struct room_data *add_room(struct room_data *rooms, short game_type, short room_identifier,
                           short ranked_room, short country_code, short min_caste,
                           short max_caste, short tournament_room);





struct room_data *load_room_list(void);
boolean save_room_list(struct room_data *rooms, char *filename);
void list_room_templates(struct room_data *rooms);
struct room_data *delete_room_template(struct room_data *rooms, short game_type, short room_identifier);
struct room_data *add_or_update_room(struct room_data *rooms, short game_type, short room_identifier,
                                     short ranked_room, short country_code, short minimum_caste,
                                     short maximum_caste, short tournament_room);

#endif
