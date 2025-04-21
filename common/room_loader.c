#include "room_loader.h"
#include "cJSON.h"
#include "../users_new/room_list_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PYTHON_ROOM_SCRIPT "python3 /src/spaghetti/load_rooms.py"
#define ROOM_JSON_BUFFER_SIZE 8192

// Load from static rooms.json file (preferred for performance)
int load_rooms_from_json_file(char *out_json, size_t max_len, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    size_t offset = fread(out_json, 1, max_len - 1, fp);
    out_json[offset] = '\0';
    fclose(fp);
    return 1;
}

// Fallback: Load from Python script (dynamic)
int load_rooms_from_python(char *out_json, size_t max_len) {
    FILE *fp = popen(PYTHON_ROOM_SCRIPT, "r");
    if (!fp) return 0;
    size_t offset = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF && offset < max_len - 1) {
        out_json[offset++] = (char)ch;
    }
    out_json[offset] = '\0';
    return pclose(fp) == 0;
}

int parse_rooms_json(const char *json_str, struct room_data **rooms_out) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsArray(root)) return NULL;

    int count = cJSON_GetArraySize(root);
    struct room_data *rooms = calloc(count, sizeof(struct room_data));
    if (!rooms) {
        cJSON_Delete(root);
        return NULL;
    }

    for (size_t i = 0; i < count; ++i) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (!item) continue;

        cJSON *id = cJSON_GetObjectItem(item, "id");
        cJSON *name = cJSON_GetObjectItem(item, "name");
        cJSON *desc = cJSON_GetObjectItem(item, "description");

        if (id && name && desc) {
            rooms[i].room_id = id->valueint;
            strncpy(rooms[i].name, name->valuestring, MAX_ROOM_NAME_LENGTH - 1);
            strncpy(rooms[i].description, desc->valuestring, MAX_ROOM_DESC_LENGTH - 1);
        }
    }

    cJSON_Delete(root);

    // removed old room_list_t allocation, now handled with struct room_data *
    if (!list) {
        free(rooms);
        return NULL;
    }

    list->rooms = rooms;
    list->count = count;
    return list;
}

void free_room_list(room_list_t *list) {
    if (!list) return;
    free(list->rooms);
    free(list);
}
