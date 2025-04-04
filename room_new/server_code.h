#ifndef SERVER_CODE_H
#define SERVER_CODE_H

#include "cseries.h"
#include "room_packets.h"

#ifdef __cplusplus
extern "C" {
#endif

    // Server code constants
#define MAX_SERVER_CODE_LENGTH 32
#define MAX_SERVER_NAME_LENGTH 64

// Server code structure
    typedef struct {
        char code[MAX_SERVER_CODE_LENGTH];
        char name[MAX_SERVER_NAME_LENGTH];
        int platform_type;
        int country_code;
        int max_players;
        int current_players;
        int game_type;
        int map_id;
        int difficulty;
        int time_limit;
        int score_limit;
        int team_play;
        int friendly_fire;
        int auto_balance;
        int password_protected;
    } server_code_t;

    // Function declarations
    int server_code_init(server_code_t* server);
    int server_code_set_code(server_code_t* server, const char* code);
    int server_code_set_name(server_code_t* server, const char* name);
    int server_code_set_platform(server_code_t* server, int platform_type);
    int server_code_set_country(server_code_t* server, int country_code);
    int server_code_set_max_players(server_code_t* server, int max_players);
    int server_code_set_current_players(server_code_t* server, int current_players);
    int server_code_set_game_type(server_code_t* server, int game_type);
    int server_code_set_map(server_code_t* server, int map_id);
    int server_code_set_difficulty(server_code_t* server, int difficulty);
    int server_code_set_time_limit(server_code_t* server, int time_limit);
    int server_code_set_score_limit(server_code_t* server, int score_limit);
    int server_code_set_team_play(server_code_t* server, int team_play);
    int server_code_set_friendly_fire(server_code_t* server, int friendly_fire);
    int server_code_set_auto_balance(server_code_t* server, int auto_balance);
    int server_code_set_password_protected(server_code_t* server, int password_protected);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_CODE_H */