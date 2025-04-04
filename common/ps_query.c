#include "ps_query.h"
#include <string.h>

int ps_query_init(ps_query_t* query) {
    if (!query) return -1;

    memset(query, 0, sizeof(ps_query_t));
    query->country_code = -1;
    query->platform_type = -1;
    query->min_rank = -1;
    query->max_rank = -1;
    query->online_only = 0;

    return 0;
}

int ps_query_set_username(ps_query_t* query, const char* username) {
    if (!query || !username) return -1;

    strncpy(query->username, username, MAX_USERNAME_LENGTH - 1);
    query->username[MAX_USERNAME_LENGTH - 1] = '\0';

    return 0;
}

int ps_query_set_country(ps_query_t* query, int country_code) {
    if (!query) return -1;

    query->country_code = country_code;
    return 0;
}

int ps_query_set_platform(ps_query_t* query, int platform_type) {
    if (!query) return -1;

    query->platform_type = platform_type;
    return 0;
}

int ps_query_set_rank_range(ps_query_t* query, int min_rank, int max_rank) {
    if (!query) return -1;

    query->min_rank = min_rank;
    query->max_rank = max_rank;
    return 0;
}

int ps_query_set_online_only(ps_query_t* query, int online_only) {
    if (!query) return -1;

    query->online_only = online_only;
    return 0;
}

int ps_query_execute(ps_query_t* query, void* results, size_t* num_results) {
    if (!query || !results || !num_results) return -1;

    // TODO: Implement actual search logic
    *num_results = 0;
    return 0;
}