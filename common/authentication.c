/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

 
 /*
  * The metaserver code changes that fall outside the original Bungie.net metaserver code 
  * license were written and are copyright 2002, 2003 of the following individuals:
  *
  * Copyright (c) 2002, 2003 Alan Wagner
  * Copyright (c) 2002 Vishvananda Ishaya
  * Copyright (c) 2003 Bill Keirstead
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  * SUCH DAMAGE.
  *
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
	// ALAN Begin: Added Code
	// Bungie removed token generation, you'll have to create your own
	char * p = token;
	memcpy(p, &user_id, sizeof(user_id));
	// ALAN End

	return;
}

boolean authenticate_token(
	long client_address, 
	long current_time, 
	long *user_id,
	authentication_token token)
{
	// ALAN Begin
//	boolean valid= FALSE;
	boolean valid= TRUE;
	// ALAN End


	// ALAN Begin: Added Code
	// Bungie removed token authentication, you'll have to create your own
	char * p = token;
	memcpy(user_id, p, sizeof(long));
	// ALAN End


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


