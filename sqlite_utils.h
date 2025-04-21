// sqlite_utils.h
#ifndef SQLITE_UTILS_H
#define SQLITE_UTILS_H
#include <sqlite3.h>

// Open DB and create all tables
int initialize_all_tables(sqlite3 **db);

// Exec a SQL statement, print error if any
int exec_sql(sqlite3 *db, const char *sql);

#endif // SQLITE_UTILS_H
