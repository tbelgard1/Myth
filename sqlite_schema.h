// sqlite_schema.h
// Contains SQL table creation statements for all persistent Myth server features.

#ifndef SQLITE_SCHEMA_H
#define SQLITE_SCHEMA_H

#define SQL_CREATE_USERS_TABLE \
    "CREATE TABLE IF NOT EXISTS users (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "username TEXT UNIQUE NOT NULL, " \
    "password TEXT NOT NULL, " \
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);"

#define SQL_CREATE_BUDDIES_TABLE \
    "CREATE TABLE IF NOT EXISTS buddies (" \
    "user_id INTEGER, " \
    "buddy_id INTEGER, " \
    "added_at DATETIME DEFAULT CURRENT_TIMESTAMP, " \
    "PRIMARY KEY(user_id, buddy_id));"

#define SQL_CREATE_GAMES_TABLE \
    "CREATE TABLE IF NOT EXISTS games (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "room_id INTEGER, " \
    "status TEXT, " \
    "start_time DATETIME, " \
    "end_time DATETIME, " \
    "winner_id INTEGER);"

#define SQL_CREATE_GAME_LOGS_TABLE \
    "CREATE TABLE IF NOT EXISTS game_logs (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "game_id INTEGER, " \
    "event_type TEXT, " \
    "event_data TEXT, " \
    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);"

#define SQL_CREATE_ROOMS_TABLE \
    "CREATE TABLE IF NOT EXISTS rooms (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "name TEXT, " \
    "owner_id INTEGER, " \
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, " \
    "status TEXT);"

#define SQL_CREATE_ROOM_MEMBERS_TABLE \
    "CREATE TABLE IF NOT EXISTS room_members (" \
    "room_id INTEGER, " \
    "user_id INTEGER, " \
    "joined_at DATETIME DEFAULT CURRENT_TIMESTAMP, " \
    "PRIMARY KEY(room_id, user_id));"

#define SQL_CREATE_MATCHMAKING_TABLE \
    "CREATE TABLE IF NOT EXISTS matchmaking_queue (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "user_id INTEGER, " \
    "game_type TEXT, " \
    "joined_at DATETIME DEFAULT CURRENT_TIMESTAMP, " \
    "status TEXT);"

#define SQL_CREATE_WEB_SESSIONS_TABLE \
    "CREATE TABLE IF NOT EXISTS web_sessions (" \
    "id INTEGER PRIMARY KEY AUTOINCREMENT, " \
    "user_id INTEGER, " \
    "session_token TEXT, " \
    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, " \
    "expires_at DATETIME);"

#endif // SQLITE_SCHEMA_H
