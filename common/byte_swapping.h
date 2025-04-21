/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __BYTE_SWAPPING_H__
#define __BYTE_SWAPPING_H__

/* ---------- constants */

enum // byte_swap_code constants
{
	_byte= 1,
	_2byte= -2,
	_4byte= -4,
	
	_begin_bs_array= -100,
	_end_bs_array= -101
};

/* ---------- macros */

#define SWAP2(q) ((((unsigned short)(q))>>8) | ((((unsigned short)(q))<<8)&0xff00))
#define SWAP4(q) (((((unsigned long) (q)))>>24) | ((((unsigned long) (q))>>8)&0xff00) | ((((unsigned long) (q))<<8)&0xff0000) | ((((unsigned long) (q))<<24)&0xff000000))

/* ---------- types */

typedef short byte_swap_code;

/* ---------- prototypes/BYTE_SWAPPING.H */

void byte_swap_memory(char *name, void *memory, int count, byte_swap_code code);
void byte_swap_data(char *name, void *data, int data_size, int data_count, byte_swap_code *codes);
void byte_swap_move(char *name, void *destination, void *source, int data_size, int data_count, byte_swap_code *codes);

#endif // __BYTE_SWAPPING_H__
