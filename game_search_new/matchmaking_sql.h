// matchmaking_sql.h
#ifndef MATCHMAKING_SQL_H
#define MATCHMAKING_SQL_H
#include <sqlite3.h>

// Matchmaking queue CRUD
int enqueue_matchmaking(sqlite3 *db, int user_id, const char *game_type, const char *status);
int dequeue_matchmaking(sqlite3 *db, int id);
// ... more as needed

#endif // MATCHMAKING_SQL_H
