// room_sql.h
#ifndef ROOM_SQL_H
#define ROOM_SQL_H
#include <sqlite3.h>

// Game CRUD
int sql_create_game(sqlite3 *db, int room_id, const char *status, int winner_id);
// ... more as needed

// Game log CRUD
int log_game_event(sqlite3 *db, int game_id, const char *event_type, const char *event_data);
// ... more as needed

// Room CRUD
int create_room(sqlite3 *db, const char *name, int owner_id, const char *status);
// ... more as needed

// Room member CRUD
int add_room_member(sqlite3 *db, int room_id, int user_id);
// ... more as needed

#endif // ROOM_SQL_H
