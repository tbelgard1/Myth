/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"

#include <string.h>
#include <ctype.h>

#include <stdarg.h>

/* ------ code */

int strnlen(
	char *string,
	int maximum_string_length)
{
	int string_length= 0;
	
	while (string_length<maximum_string_length && *string++) string_length+= 1;
	
	return string_length;
}

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

