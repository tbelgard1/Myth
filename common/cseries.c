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

#include "cseries.h"

#include <string.h>
#include <ctype.h>

#include <stdarg.h>

/* ------ code */

// ALAN Begin: CYGWIN doesn't like this definition
#ifndef CYGWIN
	int strnlen(
		char *string,
		int maximum_string_length)
	{
		int string_length= 0;
	
		while (string_length<maximum_string_length && *string++) string_length+= 1;
	
		return string_length;
	}
#endif
// ALAN End

char *strupr(
	char *string)
{
	char *p;
	
	for (p= string; *p; p++) *p= toupper(*p);
	
	return string;
}

char *strlwr(
	char *string)
{
	char *p;
	
	for (p= string; *p; p++) *p= tolower(*p);
	
	return string;
}

char *strnupr(
	char *string,
	int n)
{
	char *p;
	
	for (p= string; *p && n-->=0; p++) *p= toupper(*p);
	
	return string;
}

char *strnlwr(
	char *string,
	int n)
{
	char *p;
	
	for (p= string; *p && n-->=0; p++) *p= tolower(*p);
	
	return string;
}

