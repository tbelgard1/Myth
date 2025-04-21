// web_session_sql.h
#ifndef WEB_SESSION_SQL_H
#define WEB_SESSION_SQL_H
#include <sqlite3.h>

// Web session CRUD
int create_web_session(sqlite3 *db, int user_id, const char *session_token, const char *expires_at);
int validate_web_session(sqlite3 *db, const char *session_token);
// ... more as needed

#endif // WEB_SESSION_SQL_H
