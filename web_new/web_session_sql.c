// web_session_sql.c
#include "web_session_sql.h"
#include "../sqlite_utils.h"
#include <stdio.h>

int create_web_session(sqlite3 *db, int user_id, const char *session_token, const char *expires_at) {
    const char *sql = "INSERT INTO web_sessions (user_id, session_token, expires_at) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, session_token, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, expires_at, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int validate_web_session(sqlite3 *db, const char *session_token) {
    const char *sql = "SELECT COUNT(*) FROM web_sessions WHERE session_token = ? AND expires_at > datetime('now');";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, session_token, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    int valid = 0;
    if (rc == SQLITE_ROW) valid = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return valid > 0;
}
