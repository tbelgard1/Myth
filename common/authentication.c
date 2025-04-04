/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

// this file has been gutted :)
#include "cseries.h"
#include <string.h>

#ifdef linux
#include <time.h>
#endif

#include "authentication.h"

/* -------- code */

#ifdef SERVER
static authentication_token global_guest_token= {0};

long get_current_time(
	void)
{
	return time(NULL);
}

void test_authentication_tokens(
	void)
{
	return;
}

void get_guest_user_token(
	authentication_token token)
{
	memcpy(token, global_guest_token, sizeof(authentication_token));

	return;
}

boolean guest_token(
	authentication_token *token)
{
	return (memcmp(token, global_guest_token, sizeof(authentication_token))==0);
}

void generate_authentication_token(
	long client_address, 
	long user_id,
	long expiration_time, 
	authentication_token token)
{
	return;
}

boolean authenticate_token(
	long client_address, 
	long current_time, 
	long *user_id,
	authentication_token token)
{
	boolean valid= FALSE;
	
	return valid;
}

void get_random_salt(
	byte *salt)
{
	int index;
	static boolean first_time= TRUE;
	
	if(first_time)
	{
		srand(machine_tick_count());
		first_time= FALSE;
	}

	for(index= 0; index<MAXIMUM_SALT_SIZE; ++index)
	{
		salt[index]= (byte) random();
	}
	
	return;
}
#endif

void encrypt_password(
	char *password, 
	unsigned char *salt, 
	char *result,
	short type)
{
	strcpy(result, password);
	
	return;
}


