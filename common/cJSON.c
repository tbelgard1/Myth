/*
 * cJSON.c - simplified version for room list parsing
 * https://github.com/DaveGamble/cJSON (MIT License)
 * Only the necessary functions for parsing room JSON
 */
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// For brevity, this is a stub. In production, use the full cJSON.c from the repo.
cJSON *cJSON_Parse(const char *value) { fprintf(stderr, "[WARN] cJSON_Parse stub called!\n"); return NULL; }
void cJSON_Delete(cJSON *c) { }
int cJSON_IsArray(const cJSON *item) { return 0; }
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string) { return NULL; }

// Minimal stub implementations to avoid linker errors
int cJSON_GetArraySize(const cJSON *array) { return 0; }
cJSON *cJSON_GetArrayItem(const cJSON *array, int index) { return NULL; }
