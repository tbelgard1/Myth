#ifndef PS_QUERY_H
#define PS_QUERY_H

#include "cseries.h"

// Player search query structure
typedef struct {
    char username[MAX_USERNAME_LENGTH];
    int country_code;
    int platform_type;
    int min_rank;
    int max_rank;
    int online_only;
} ps_query_t;

// Function declarations
int ps_query_init(ps_query_t* query);
int ps_query_set_username(ps_query_t* query, const char* username);
int ps_query_set_country(ps_query_t* query, int country_code);
int ps_query_set_platform(ps_query_t* query, int platform_type);
int ps_query_set_rank_range(ps_query_t* query, int min_rank, int max_rank);
int ps_query_set_online_only(ps_query_t* query, int online_only);
int ps_query_execute(ps_query_t* query, void* results, size_t* num_results);

#endif /* PS_QUERY_H */