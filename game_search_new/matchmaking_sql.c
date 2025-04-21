// matchmaking_sql.c
#include "matchmaking_sql.h"
#include "../sqlite_utils.h"
#include <stdio.h>

int enqueue_matchmaking(sqlite3 *db, int user_id, const char *game_type, const char *status) {
    const char *sql = "INSERT INTO matchmaking_queue (user_id, game_type, status) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, game_type, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, status, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int dequeue_matchmaking(sqlite3 *db, int id) {
    const char *sql = "DELETE FROM matchmaking_queue WHERE id = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
