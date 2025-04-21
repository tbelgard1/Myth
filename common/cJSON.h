/*
 * cJSON.h - simplified version for room list parsing
 * https://github.com/DaveGamble/cJSON (MIT License)
 * Only the necessary declarations for parsing room JSON
 */
#ifndef CJSON_H
#define CJSON_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct cJSON {
    struct cJSON *next, *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;

cJSON *cJSON_Parse(const char *value);
void cJSON_Delete(cJSON *c);
int cJSON_IsArray(const cJSON *item);
cJSON *cJSON_GetObjectItem(const cJSON *object, const char *string);
#define cJSON_ArrayForEach(element, array) \
    for(element = (array ? (array)->child : NULL); element != NULL; element = element->next)

#ifdef __cplusplus
}
#endif
#endif // CJSON_H
