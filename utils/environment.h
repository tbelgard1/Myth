/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

// Environment variable names
#define BUNGIE_NET_ADMINISTRATOR "BUNGIE_NET_ADMINISTRATOR"
#define METASERVER_ROOT_DIR "METASERVER_ROOT_DIR"
#define MOTD_FILE_NAME "MOTD_FILE_NAME"
#define UPGRADE_PORT "UPGRADE_PORT"
#define USERD_HOST "USERD_HOST"
#define USERD_PORT "USERD_PORT"
#define USERD_ROOM_PORT "USERD_ROOM_PORT"
#define USERD_WEB_PORT "USERD_WEB_PORT"
#define USERD_US_PORT "USERD_US_PORT"
#define DB_DIRECTORY "DB_DIRECTORY"
#define ORDERS_DB_FILE_NAME "ORDERS_DB_FILE_NAME"
#define USERS_DB_FILE_NAME "USERS_DB_FILE_NAME"
#define UPDATE_FILE_NAME "UPDATE_FILE_NAME"
#define UPDATE_DIRECTORY "UPDATE_DIRECTORY"
#define LOG_DIRECTORY "LOG_DIRECTORY"
#define ROOMS_LIST_FILE "ROOMS_LIST_FILE"
#define DEFAULT_APPLE_DOUBLE_DIRECTORY "DEFAULT_APPLE_DOUBLE_DIRECTORY"
#define RESOURCE_DIRECTORY "RESOURCE_DIRECTORY"
#define PC_APP_NAME "PC_APP_NAME"
#define PC_NET_ONLY_APP_NAME "PC_NET_ONLY_APP_NAME"
#define PC_UPDATE_APP_NAME "PC_UPDATE_APP_NAME"
#define PC_NET_ONLY_UPDATE_APP_NAME "PC_NET_ONLY_UPDATE_APP_NAME"
#define MAC_APP_NAME "MAC_APP_NAME"
#define MAC_NET_ONLY_APP_NAME "MAC_NET_ONLY_APP_NAME"
#define MAC_UPDATE_APP_NAME "MAC_UPDATE_APP_NAME"
#define MAC_NET_ONLY_UPDATE_APP_NAME "MAC_NET_ONLY_UPDATE_APP_NAME"
#define PATCH_FILE_NAME "PATCH_FILE_NAME"
#define ADMIN_LOG_FILE_NAME "ADMIN_LOG_FILE_NAME"
#define UPDATE_SERVER_STATS_FILE_PATH "UPDATE_SERVER_STATS_FILE_PATH"

// Function declarations for environment-based settings
#ifndef HARDCODE_USERD_SETTINGS
char * get_bungie_net_administrator(void);
char * get_metaserver_root_dir(void);
char * get_motd_file_name(void);
char * get_upgrade_port(void);
char * get_userd_host(void);
char * get_userd_port(void);
char * get_userd_room_port(void);
char * get_userd_web_port(void);
char * get_userd_us_port(void);
char * get_db_directory(void);
char * get_orders_db_file_name(void);
char * get_users_db_file_name(void);
char * get_update_file_name(void);
char * get_update_directory(void);
char * get_log_directory(void);
char * get_rooms_list_file(void);
char * get_default_apple_double_directory(void);
char * get_resource_directory(void);
char * get_pc_app_name(void);
char * get_pc_net_only_app_name(void);
char * get_pc_update_app_name(void);
char * get_pc_net_only_update_app_name(void);
char * get_mac_app_name(void);
char * get_mac_net_only_app_name(void);
char * get_mac_update_app_name(void);
char * get_mac_net_only_update_app_name(void);
char * get_patch_file_name(void);
char * get_admin_log_file_name(void);
char * get_update_server_stats_file_path(void);
#endif

// Function declarations for hardcoded settings
#ifdef HARDCODE_USERD_SETTINGS
const char* get_bungie_net_administrator(void);
const char* get_metaserver_root_dir(void);
const char* get_motd_file_name(void);
const char* get_upgrade_port(void);
const char* get_userd_host(void);
const char* get_userd_port(void);
const char* get_userd_room_port(void);
const char* get_userd_web_port(void);
const char* get_userd_us_port(void);
const char* get_db_directory(void);
const char* get_orders_db_file_name(void);
const char* get_users_db_file_name(void);
const char* get_update_file_name(void);
const char* get_update_directory(void);
const char* get_log_directory(void);
const char* get_rooms_list_file(void);
const char* get_default_apple_double_directory(void);
const char* get_resource_directory(void);
const char* get_pc_app_name(void);
const char* get_pc_net_only_app_name(void);
const char* get_pc_update_app_name(void);
const char* get_pc_net_only_update_app_name(void);
const char* get_mac_app_name(void);
const char* get_mac_net_only_app_name(void);
const char* get_mac_update_app_name(void);
const char* get_mac_net_only_update_app_name(void);
const char* get_patch_file_name(void);
const char* get_admin_log_file_name(void);
const char* get_update_server_stats_file_path(void);
#endif

#endif // _ENVIRONMENT_H_