/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __CSERIES_H
#define __CSERIES_H

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

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
#define assert(expr)
#define vassert(expr,diag)
#define warn(expr)
#define vwarn(expr,diag)
#define pause()
#define vpause(diag)
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
#define ntohl(q) (((((unsigned long) (q)))>>24) | ((((unsigned long) (q))>>8)&0xff00) | ((((unsigned long) (q))<<8)&0xff0000) | ((((unsigned long) (q))<<24)&0xff000000))
#define htonl(q) (((((unsigned long) (q)))>>24) | ((((unsigned long) (q))>>8)&0xff00) | ((((unsigned long) (q))<<8)&0xff0000) | ((((unsigned long) (q))<<24)&0xff000000))
#define ntohs(q) ((((unsigned short)(q))>>8) | ((((unsigned short)(q))<<8)&0xff00))
#define htons(q) ((((unsigned short)(q))>>8) | ((((unsigned short)(q))<<8)&0xff00))
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
	UNSIGNED_LONG_MAX= ULONG_MAX,
	LONG_BITS= 32,

	UNSIGNED_SHORT_MAX= 65535,
	SHORT_MAX= SHRT_MAX,
	SHORT_MIN= SHRT_MIN,
	SHORT_BITS= 16,

	UNSIGNED_CHAR_MAX= UCHAR_MAX,
	CHAR_BITS= CHAR_BIT
};

/* ---------- prototypes */

int strnlen(char *string, int n);

char *strupr(char *string);
char *strnupr(char *string, int n);
char *strlwr(char *string);
char *strnlwr(char *string, int n);

#endif // __CSERIES_H__
