/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "environment.h"
#include <stdlib.h>

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

#endif // HARDCODE_USERD_SETTINGS

// Always compiled:
const char *get_users_db_file_name(void)
{
    return "users.db";
}
