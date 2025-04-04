/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "environment.h"

#ifndef HARDCODE_USERD_SETTINGS


#include "cseries.h"

#include <stdlib.h>


char *bnet_getenv(char *var); // replaces getenv()
char *bnet_getenv(char *var)
{
	char *setting;
	
	if((setting= getenv(var)) == NULL) setting= "";

	return setting;
}



char * get_bungie_net_administrator(void)
{
	return bnet_getenv(BUNGIE_NET_ADMINISTRATOR);
}

char * get_metaserver_root_dir(void)
{
	return bnet_getenv(METASERVER_ROOT_DIR);
}

char * get_motd_file_name(void)
{
	return bnet_getenv(MOTD_FILE_NAME);
}

char * get_upgrade_port(void)
{
	return bnet_getenv(UPGRADE_PORT);
}

char * get_userd_host(void)
{
	return bnet_getenv(USERD_HOST);
}

char * get_userd_port(void)
{
	return bnet_getenv(USERD_PORT);
}

char * get_userd_room_port(void)
{
	return bnet_getenv(USERD_ROOM_PORT);
}

char * get_userd_web_port(void)
{
	return bnet_getenv(USERD_WEB_PORT);
}

char * get_userd_us_port(void)
{
	return bnet_getenv(USERD_US_PORT);
}

char * get_db_directory(void)
{
	return bnet_getenv(DB_DIRECTORY);
}

char * get_orders_db_file_name(void)
{
	return bnet_getenv(ORDERS_DB_FILE_NAME);
}

char * get_users_db_file_name(void)
{
	return bnet_getenv(USERS_DB_FILE_NAME);
}

char * get_update_file_name(void)
{
	return bnet_getenv(UPDATE_FILE_NAME);
}

char * get_update_directory(void)
{
	return bnet_getenv(UPDATE_DIRECTORY);
}

char * get_log_directory(void)
{
	return bnet_getenv(LOG_DIRECTORY);
}

char * get_rooms_list_file(void)
{
	return bnet_getenv(ROOMS_LIST_FILE);
}

char * get_default_apple_double_directory(void)
{
	return bnet_getenv(DEFAULT_APPLE_DOUBLE_DIRECTORY);
}

char * get_resource_directory(void)
{
	return bnet_getenv(RESOURCE_DIRECTORY);
}

char * get_pc_app_name(void)
{
	return bnet_getenv(PC_APP_NAME);
}

char * get_pc_net_only_app_name(void)
{
	return bnet_getenv(PC_NET_ONLY_APP_NAME);
}

char * get_pc_update_app_name(void)
{
	return bnet_getenv(PC_UPDATE_APP_NAME);
}

char * get_pc_net_only_update_app_name(void)
{
	return bnet_getenv(PC_NET_ONLY_UPDATE_APP_NAME);
}

char * get_mac_app_name(void)
{
	return bnet_getenv(MAC_APP_NAME);
}

char * get_mac_net_only_app_name(void)
{
	return bnet_getenv(MAC_NET_ONLY_APP_NAME);
}

char * get_mac_update_app_name(void)
{
	return bnet_getenv(MAC_UPDATE_APP_NAME);
}

char * get_mac_net_only_update_app_name(void)
{
	return bnet_getenv(MAC_NET_ONLY_UPDATE_APP_NAME);
}

char * get_patch_file_name(void)
{
	return bnet_getenv(PATCH_FILE_NAME);
}

char * get_admin_log_file_name(void)
{
	return bnet_getenv(ADMIN_LOG_FILE_NAME);
}

char * get_update_server_stats_file_path(void)
{
	return bnet_getenv(UPDATE_SERVER_STATS_FILE_PATH);
}

#endif // HARDCODE_USERD_SETTINGS
