/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "environment.h"

#ifdef HARDCODE_USERD_SETTINGS

// Hardcoded settings for Windows environment
const char* get_bungie_net_administrator(void) { return "admin"; }
const char* get_metaserver_root_dir(void) { return "C:\\BungieNet"; }
const char* get_motd_file_name(void) { return "motd.txt"; }
const char* get_upgrade_port(void) { return "8080"; }
const char* get_userd_host(void) { return "localhost"; }
const char* get_userd_port(void) { return "9000"; }
const char* get_userd_room_port(void) { return "9001"; }
const char* get_userd_web_port(void) { return "9002"; }
const char* get_userd_us_port(void) { return "9003"; }
const char* get_db_directory(void) { return "C:\\BungieNet\\db"; }
const char* get_orders_db_file_name(void) { return "orders.db"; }
const char* get_users_db_file_name(void) { return "users.db"; }
const char* get_update_file_name(void) { return "update.txt"; }
const char* get_update_directory(void) { return "C:\\BungieNet\\updates"; }
const char* get_log_directory(void) { return "C:\\BungieNet\\logs"; }
const char* get_rooms_list_file(void) { return "rooms.lst"; }
const char* get_default_apple_double_directory(void) { return "C:\\BungieNet\\apple"; }
const char* get_resource_directory(void) { return "C:\\BungieNet\\resources"; }
const char* get_pc_app_name(void) { return "myth2.exe"; }
const char* get_pc_net_only_app_name(void) { return "myth2_net.exe"; }
const char* get_pc_update_app_name(void) { return "myth2_update.exe"; }
const char* get_pc_net_only_update_app_name(void) { return "myth2_net_update.exe"; }
const char* get_mac_app_name(void) { return "Myth II"; }
const char* get_mac_net_only_app_name(void) { return "Myth II Net"; }
const char* get_mac_update_app_name(void) { return "Myth II Update"; }
const char* get_mac_net_only_update_app_name(void) { return "Myth II Net Update"; }
const char* get_patch_file_name(void) { return "patch.dat"; }
const char* get_admin_log_file_name(void) { return "admin.log"; }
const char* get_update_server_stats_file_path(void) { return "C:\\BungieNet\\stats.txt"; }

#endif // HARDCODE_USERD_SETTINGS