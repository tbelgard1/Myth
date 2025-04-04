#include "server_code.h"
#include <string.h>

int server_code_init(server_code_t* server) {
    if (!server) return -1;

    memset(server, 0, sizeof(server_code_t));
    server->platform_type = -1;
    server->country_code = -1;
    server->max_players = 16;
    server->current_players = 0;
    server->game_type = 0;
    server->map_id = 0;
    server->difficulty = 0;
    server->time_limit = 0;
    server->score_limit = 0;
    server->team_play = 0;
    server->friendly_fire = 0;
    server->auto_balance = 0;
    server->password_protected = 0;

    return 0;
}

int server_code_set_code(server_code_t* server, const char* code) {
    if (!server || !code) return -1;

    strncpy(server->code, code, MAX_SERVER_CODE_LENGTH - 1);
    server->code[MAX_SERVER_CODE_LENGTH - 1] = '\0';

    return 0;
}

int server_code_set_name(server_code_t* server, const char* name) {
    if (!server || !name) return -1;

    strncpy(server->name, name, MAX_SERVER_NAME_LENGTH - 1);
    server->name[MAX_SERVER_NAME_LENGTH - 1] = '\0';

    return 0;
}

int server_code_set_platform(server_code_t* server, int platform_type) {
    if (!server) return -1;

    server->platform_type = platform_type;
    return 0;
}

int server_code_set_country(server_code_t* server, int country_code) {
    if (!server) return -1;

    server->country_code = country_code;
    return 0;
}

int server_code_set_max_players(server_code_t* server, int max_players) {
    if (!server) return -1;

    server->max_players = max_players;
    return 0;
}

int server_code_set_current_players(server_code_t* server, int current_players) {
    if (!server) return -1;

    server->current_players = current_players;
    return 0;
}

int server_code_set_game_type(server_code_t* server, int game_type) {
    if (!server) return -1;

    server->game_type = game_type;
    return 0;
}

int server_code_set_map(server_code_t* server, int map_id) {
    if (!server) return -1;

    server->map_id = map_id;
    return 0;
}

int server_code_set_difficulty(server_code_t* server, int difficulty) {
    if (!server) return -1;

    server->difficulty = difficulty;
    return 0;
}

int server_code_set_time_limit(server_code_t* server, int time_limit) {
    if (!server) return -1;

    server->time_limit = time_limit;
    return 0;
}

int server_code_set_score_limit(server_code_t* server, int score_limit) {
    if (!server) return -1;

    server->score_limit = score_limit;
    return 0;
}

int server_code_set_team_play(server_code_t* server, int team_play) {
    if (!server) return -1;

    server->team_play = team_play;
    return 0;
}

int server_code_set_friendly_fire(server_code_t* server, int friendly_fire) {
    if (!server) return -1;

    server->friendly_fire = friendly_fire;
    return 0;
}

int server_code_set_auto_balance(server_code_t* server, int auto_balance) {
    if (!server) return -1;

    server->auto_balance = auto_balance;
    return 0;
}

int server_code_set_password_protected(server_code_t* server, int password_protected) {
    if (!server) return -1;

    server->password_protected = password_protected;
    return 0;
}