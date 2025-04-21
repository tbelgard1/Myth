/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H


#ifdef HARDCODE_USERD_SETTINGS
	
	#ifdef RUNNING_LOCALLY
		#warning USERD_HOST set as localhost
		#define USERD_HOST "0.0.0.0"
	#endif
	
	#ifdef BN2_FULLVERSION
		#define BUNGIE_NET_ADMINISTRATOR			"nobody"
		#define METASERVER_ROOT_DIR					"/tmp"
		#define MOTD_FILE_NAME						"motd"
		#define UPDATE_PORT "7982" // update
		#ifndef USERD_HOST
			#define USERD_HOST "127.0.0.1"
		#endif
		#define USERD_PORT "6321" // userd
		#define USERD_ROOM_PORT "6322" // roomd
		#define USERD_WEB_PORT "6323" // webd
		#define USERD_GAMESearch_PORT "6324" // gamesearchd
		#define USERD_US_PORT "7981" // player search
		#define DB_DIRECTORY						"/tmp"
		#define ORDERS_DB_FILE_NAME					"/tmp/ORDERS_DB"
		#define USERS_DB_FILE_NAME "/tmp/USERS_DB"
		#define UPDATE_FILE_NAME					"/tmp/UPDATE_DB"
		#define UPDATE_DIRECTORY					"/tmp/"
		#define LOG_DIRECTORY						"/tmp/"
		#define ROOMS_LIST_FILE						"rooms.lst"
		#define DEFAULT_APPLE_DOUBLE_DIRECTORY		".AppleDouble"
		#define RESOURCE_DIRECTORY					"System Folder"
		#define PC_APP_NAME							"Myth2.exe"
		#define PC_NET_ONLY_APP_NAME				"obsolete"
		#define PC_UPDATE_APP_NAME					"obsolete.exe"
		#define PC_NET_ONLY_UPDATE_APP_NAME			"obsolete"
		#define MAC_APP_NAME						"obsolete"
		#define MAC_NET_ONLY_APP_NAME				"obsolete"
		#define MAC_UPDATE_APP_NAME					"obsolete"
		#define MAC_NET_ONLY_UPDATE_APP_NAME		"obsolete"
		#define PATCH_FILE_NAME						"obsolete"
		#define ADMIN_LOG_FILE_NAME					"adminlog.txt"
		#define UPDATE_SERVER_STATS_FILE_PATH		"./"
	#elif defined(BN2_DEMOVERSION)
		#define BUNGIE_NET_ADMINISTRATOR			"nobody"
		#define METASERVER_ROOT_DIR					"/tmp"
		#define MOTD_FILE_NAME						"motd"
		#define UPDATE_PORT "7982" // update
		#ifndef USERD_HOST
			#warning USERD_HOST set to demo server
			#define USERD_HOST "0.0.0.0"
		#endif
		#define USERD_PORT						"6321"
		#define USERD_ROOM_PORT						"6322"
		#define USERD_WEB_PORT						"6323"
		#define USERD_GAMESearch_PORT				"6324"
		#define USERD_US_PORT						"7981"
		#define USERD_WEB_PORT						"6322"						"6332"
		#define USERD_US_PORT						"-1"						"6334"
		#define DB_DIRECTORY						"/tmp"
		#define ORDERS_DB_FILE_NAME					"/tmp/ORDERS_DB"
		#define USERS_DB_FILE_NAME "/tmp/USERS_DB"
		#define UPDATE_FILE_NAME					"/tmp/UPDATE_DB"
		#define UPDATE_DIRECTORY					"/tmp/"
		#define LOG_DIRECTORY						"/tmp/"
		#define ROOMS_LIST_FILE						"rooms.lst"
		#define DEFAULT_APPLE_DOUBLE_DIRECTORY		".AppleDouble"
		#define RESOURCE_DIRECTORY					"System Folder"
		#define PC_APP_NAME							"Myth2.exe"
		#define PC_NET_ONLY_APP_NAME				"obsolete"
		#define PC_UPDATE_APP_NAME					"obsolete.exe"
		#define PC_NET_ONLY_UPDATE_APP_NAME			"obsolete"
		#define MAC_APP_NAME						"obsolete"
		#define MAC_NET_ONLY_APP_NAME				"obsolete"
		#define MAC_UPDATE_APP_NAME					"obsolete"
		#define MAC_NET_ONLY_UPDATE_APP_NAME		"obsolete"
		#define PATCH_FILE_NAME						"obsolete"
		#define ADMIN_LOG_FILE_NAME					"adminlog.txt"
		#define UPDATE_SERVER_STATS_FILE_PATH		"./"
	#endif
	
	#define get_bungie_net_administrator()			BUNGIE_NET_ADMINISTRATOR
	#define get_metaserver_root_dir()				METASERVER_ROOT_DIR
	#define get_motd_file_name()					MOTD_FILE_NAME
	#define get_upgrade_port()						UPGRADE_PORT
	#define get_userd_host()						USERD_HOST
	#define get_userd_port()						USERD_PORT
	#define get_userd_room_port()					USERD_ROOM_PORT
	#define get_userd_web_port()					USERD_WEB_PORT
	#define get_userd_us_port()						USERD_US_PORT
	#define get_db_directory()						DB_DIRECTORY
	#define get_orders_db_file_name()				ORDERS_DB_FILE_NAME
	#define get_users_db_file_name() USERS_DB_FILE_NAME
	#define get_update_file_name()					UPDATE_FILE_NAME
	#define get_update_directory()					UPDATE_DIRECTORY
	#define get_log_directory()						LOG_DIRECTORY
	#define get_rooms_list_file()					ROOMS_LIST_FILE
	#define get_default_apple_double_directory()	DEFAULT_APPLE_DOUBLE_DIRECTORY
	#define get_resource_directory()				RESOURCE_DIRECTORY
	#define get_pc_app_name()						PC_APP_NAME
	#define get_pc_net_only_app_name()				PC_NET_ONLY_APP_NAME
	#define get_pc_update_app_name()				PC_UPDATE_APP_NAME
	#define get_pc_net_only_update_app_name()		PC_NET_ONLY_UPDATE_APP_NAME
	#define get_mac_app_name()						MAC_APP_NAME
	#define get_mac_net_only_app_name()				MAC_NET_ONLY_APP_NAME
	#define get_mac_update_app_name()				MAC_UPDATE_APP_NAME
	#define get_mac_net_only_update_app_name()		MAC_NET_ONLY_UPDATE_APP_NAME
	#define get_patch_file_name()					PATCH_FILE_NAME
	#define get_admin_log_file_name()				ADMIN_LOG_FILE_NAME
	#define get_update_server_stats_file_path()		UPDATE_SERVER_STATS_FILE_PATH

#else

	#define	BUNGIE_NET_ADMINISTRATOR		"BUNGIE_NET_ADMINISTRATOR"
	#define	METASERVER_ROOT_DIR				"METASERVER_ROOT_DIR"
	#define	MOTD_FILE_NAME					"MOTD_FILE_NAME"
	#define UPGRADE_PORT					"UPGRADE_PORT"
	#define	USERD_HOST						"USERD_HOST"
	#define	USERD_PORT						"USERD_PORT"
	#define USERD_ROOM_PORT						"3453"						"3453" // Changed to match Myth II client expectation					"USERD_ROOM_PORT"
	#define USERD_WEB_PORT						"6322"					"USERD_WEB_PORT"
	#define USERD_US_PORT						"-1"					"USERD_US_PORT"
	#define DB_DIRECTORY					"DB_DIRECTORY"
	#define ORDERS_DB_FILE_NAME				"ORDERS_DB_FILE_NAME"
	#define UPDATE_FILE_NAME				"UPDATE_FILE_NAME"
	#define UPDATE_DIRECTORY				"UPDATE_DIRECTORY"
	#define	LOG_DIRECTORY					"LOG_DIRECTORY"
	#define ROOMS_LIST_FILE					"ROOMS_LIST_FILE"
	#define	DEFAULT_APPLE_DOUBLE_DIRECTORY	"DEFAULT_APPLE_DOUBLE_DIRECTORY"
	#define RESOURCE_DIRECTORY				"RESOURCE_DIRECTORY"
	#define	PC_APP_NAME						"PC_APP_NAME"
	#define PC_NET_ONLY_APP_NAME			"PC_NET_ONLY_APP_NAME"
	#define	PC_UPDATE_APP_NAME				"PC_UPDATE_APP_NAME"
	#define	PC_NET_ONLY_UPDATE_APP_NAME		"PC_NET_ONLY_UPDATE_APP_NAME"
	#define	MAC_APP_NAME					"MAC_APP_NAME"
	#define	MAC_NET_ONLY_APP_NAME			"MAC_NET_ONLY_APP_NAME"
	#define MAC_UPDATE_APP_NAME				"MAC_UPDATE_APP_NAME"
	#define	MAC_NET_ONLY_UPDATE_APP_NAME	"MAC_NET_ONLY_UPDATE_APP_NAME"
	#define PATCH_FILE_NAME					"PATCH_FILE_NAME"
	#define	ADMIN_LOG_FILE_NAME				"ADMIN_LOG_FILE_NAME"
	#define	UPDATE_SERVER_STATS_FILE_PATH	"UPDATE_SERVER_STATS_FILE_PATH"
	
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
const char * get_users_db_file_name(void);
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

#endif // HARDCODE_USERD_SETTINGS



#endif // __ENVIRONMENT_H
