// room_sql.c
#include "room_sql.h"
#include "../sqlite_utils.h"
#include <stdio.h>

int sql_create_game(sqlite3 *db, int room_id, const char *status, int winner_id) {
    const char *sql = "INSERT INTO games (room_id, status, winner_id, start_time) VALUES (?, ?, ?, datetime('now'));";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_text(stmt, 2, status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, winner_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int log_game_event(sqlite3 *db, int game_id, const char *event_type, const char *event_data) {
    const char *sql = "INSERT INTO game_logs (game_id, event_type, event_data) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, game_id);
    sqlite3_bind_text(stmt, 2, event_type, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, event_data, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int create_room(sqlite3 *db, const char *name, int owner_id, const char *status) {
    const char *sql = "INSERT INTO rooms (name, owner_id, status) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, owner_id);
    sqlite3_bind_text(stmt, 3, status, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int add_room_member(sqlite3 *db, int room_id, int user_id) {
    const char *sql = "INSERT INTO room_members (room_id, user_id) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, room_id);
    sqlite3_bind_int(stmt, 2, user_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}
