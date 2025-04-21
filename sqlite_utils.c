// sqlite_utils.c
#include "sqlite_utils.h"
#include "sqlite_schema.h"
#include <stdio.h>

int exec_sql(sqlite3 *db, const char *sql) {
    char *errmsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return 0;
    }
    return 1;
}

int initialize_all_tables(sqlite3 **db) {
    int rc = sqlite3_open("myth.db", db);
    if (rc) {
        fprintf(stderr, "Can't open myth.db: %s\n", sqlite3_errmsg(*db));
        return 0;
    }
    if (!exec_sql(*db, SQL_CREATE_USERS_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_BUDDIES_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_GAMES_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_GAME_LOGS_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_ROOMS_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_ROOM_MEMBERS_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_MATCHMAKING_TABLE)) return 0;
    if (!exec_sql(*db, SQL_CREATE_WEB_SESSIONS_TABLE)) return 0;
    return 1;
}
