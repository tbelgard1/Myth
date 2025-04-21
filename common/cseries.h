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

#ifndef __CSERIES_H
#define __CSERIES_H

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

// ALAN Begin: added headers
#include <sys/time.h>
// ALAN End

#undef MIN
#undef MAX
#undef ABS

/* ---------- constants */

#undef TRUE
#undef FALSE

#define TRUE 1
#define FALSE 0

#define NONE -1

#define KILO 1024
#define MEG (KILO*KILO)
#define GIG (KILO*MEG)

#define MACHINE_TICKS_PER_SECOND 1

/* ---------- macros */

#define halt()
#define vhalt(diag)

// ALAN Begin: added macro defs to quiet compiler warnings
#define assert(expr) if (!(expr)) { } 
#define vassert(expr,diag) if (!(expr)) { } 
//#define assert(expr)
//#define vassert(expr,diag)
// ALAN End

#define warn(expr)
#define vwarn(expr,diag)

// ALAN Begin: CYGWIN and Red Hat doesn't like this definition
#ifndef linux
	#define pause()
#endif
// ALAN End

#define vpause(diag)
#define KILO 1024
#define NONE -1
#define machine_tick_count() ((unsigned long)time(NULL))
#define require(b,e) do { if (!(b)) goto e; } while (0)

#define SGN(x) ((x)?((x)<0?-1:1):0)

#define ABS(x) ((x>=0) ? (x) : -(x))

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define FLOOR(n,floor) ((n)<(floor)?(floor):(n))
#define CEILING(n,ceiling) ((n)>(ceiling)?(ceiling):(n))
#define PIN(n,floor,ceiling) ((n)<(floor) ? (floor) : CEILING(n,ceiling))

#define FLAG(b) (1<<(b))

#define TEST_FLAG(f, b) ((f)&FLAG(b))
#define SWAP_FLAG(f, b) ((f)^=FLAG(b))
#define SET_FLAG(f, b, v) ((v) ? ((f)|=FLAG(b)) : ((f)&=~FLAG(b)))
#define FLAG_RANGE(first_bit, last_bit) ((FLAG((last_bit)+1-(first_bit))-1)<<(first_bit))

#define UNUSED_PARAMETER(x) (x=x)

#define HIGH_WORD(n) (((n)>>16)&0xffff)
#define LOW_WORD(n) ((n)&0xffff)

#ifdef little_endian

// ALAN Begin: Cygwin's and Red Hat's versions work just fine
#ifndef linux
	#define ntohl(q) (((((unsigned long) (q)))>>24) | ((((unsigned long) (q))>>8)&0xff00) | ((((unsigned long) (q))<<8)&0xff0000) | ((((unsigned long) (q))<<24)&0xff000000))
	#define htonl(q) (((((unsigned long) (q)))>>24) | ((((unsigned long) (q))>>8)&0xff00) | ((((unsigned long) (q))<<8)&0xff0000) | ((((unsigned long) (q))<<24)&0xff000000))
	#define ntohs(q) ((((unsigned short)(q))>>8) | ((((unsigned short)(q))<<8)&0xff00))
	#define htons(q) ((((unsigned short)(q))>>8) | ((((unsigned short)(q))<<8)&0xff00))
#endif
// ALAN End

#else
#ifndef ntohl
	#define ntohl(q) (q)
	#define htonl(q) (q)
	#define ntohs(q) (q)
	#define htons(q) (q)
#endif
#endif

/* ---------- fixed math */

typedef long fixed;
typedef unsigned short fixed_fraction;

typedef short short_fixed;
typedef unsigned char short_fixed_fraction;

enum
{
	// 16.16 fixed
	FIXED_FRACTIONAL_BITS= 16,
	FIXED_ONE= 1<<FIXED_FRACTIONAL_BITS,
	FIXED_ONE_HALF= FIXED_ONE/2,

	// 8.8 short_fixed
	SHORT_FIXED_FRACTIONAL_BITS= 8,
	SHORT_FIXED_ONE= 1<<SHORT_FIXED_FRACTIONAL_BITS,
	SHORT_FIXED_ONE_HALF= SHORT_FIXED_ONE/2
};

#define FIXED_TO_SHORT_FIXED(f) ((f)>>(FIXED_FRACTIONAL_BITS-SHORT_FIXED_FRACTIONAL_BITS))
#define SHORT_FIXED_TO_FIXED(f) ((f)<<(FIXED_FRACTIONAL_BITS-SHORT_FIXED_FRACTIONAL_BITS))

#define FIXED_TO_FLOAT(f) (((double)(f))/FIXED_ONE)
#define FLOAT_TO_FIXED(f) ((fixed)((f)*FIXED_ONE))

#define INTEGER_TO_FIXED(s) (((fixed)(s))<<FIXED_FRACTIONAL_BITS)
#define FIXED_TO_INTEGER(f) ((f)>>FIXED_FRACTIONAL_BITS)
#define FIXED_TO_INTEGER_ROUND(f) FIXED_TO_INTEGER((f)+FIXED_ONE_HALF)
#define FIXED_FRACTIONAL_PART(f) ((f)&(FIXED_ONE-1))

#define SHORT_FIXED_TO_FLOAT(f) (((double)(f))/SHORT_FIXED_ONE)
#define FLOAT_TO_SHORT_FIXED(f) ((short_fixed)((f)*SHORT_FIXED_ONE))

#define INTEGER_TO_SHORT_FIXED(s) (((short_fixed)(s))<<SHORT_FIXED_FRACTIONAL_BITS)
#define SHORT_FIXED_TO_INTEGER(f) ((f)>>SHORT_FIXED_FRACTIONAL_BITS)
#define SHORT_FIXED_TO_INTEGER_ROUND(f) SHORT_FIXED_TO_INTEGER((f)+SHORT_FIXED_ONE_HALF)
#define SHORT_FIXED_FRACTIONAL_PART(f) ((f)&(SHORT_FIXED_ONE-1))

/* ---------- types */

typedef unsigned short word;
typedef unsigned char byte;
typedef int boolean;

/* ---------- limits */

enum
{
	LONG_BITS= 32,
	SHORT_BITS= 16,
	CHAR_BITS= 8
};

#define UNSIGNED_LONG_MAX 4294967295UL
#define UNSIGNED_SHORT_MAX 65535
#define SHORT_MAX 32767
#define SHORT_MIN (-32768)
#define UNSIGNED_CHAR_MAX 255

/* ---------- prototypes */

// ALAN Begin: CYGWIN doesn't like this prototype
#ifndef CYGWIN
	int strnlen(char *string, int n);
#endif
// ALAN End

char *strupr(char *string);
char *strnupr(char *string, int n);
char *strlwr(char *string);
char *strnlwr(char *string, int n);

#endif // __CSERIES_H__
