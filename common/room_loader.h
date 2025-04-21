#ifndef ROOM_LOADER_H
#define ROOM_LOADER_H

#ifndef ROOM_LOADER_H
#define ROOM_LOADER_H

#include <stddef.h>
#include "../users_new/room_list_file.h"
#include "cJSON.h"

#define ROOM_JSON_BUFFER_SIZE 8192

// Function declarations, matching room_list_file.h
int load_rooms_from_json_file(char *buf, size_t buf_size, const char *filename);
int parse_rooms_json(const char *json_str, struct room_data **rooms_out);
void free_room_list(room_list_t *list);

#endif // ROOM_LOADER_H

#endif // ROOM_LOADER_H
