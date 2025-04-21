/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#include "cseries.h"
#include <string.h>

#include "byte_swapping.h"

/* ---------- private prototypes */

static void _byte_swap_data(char *name, void *data, byte_swap_code *codes, int *total_size_in_bytes, int *total_size_in_codes);

/* ---------- code */

void byte_swap_move(
	char *name,
	void *destination,
	void *source,
	int data_size,
	int data_count,
	byte_swap_code *codes)
{
	assert(destination && source);
	
	memmove(destination, source, data_size);
	byte_swap_data(name, destination, data_size, data_count, codes);
	
	return;
}

void byte_swap_data(
	char *name,
	void *data,
	int data_size,
	int data_count,
	byte_swap_code *codes)
{
	int i;

	assert(codes);
	
	if (data)
	{
		for (i= 0; i<data_count; ++i)
		{
			_byte_swap_data(name, (byte *)data + i*data_size, codes, (int *) NULL, (int *) NULL);
		}
	}
	
	return;
}

void byte_swap_memory(
	char *name,
	void *memory,
	int count,
	byte_swap_code code)
{
	byte_swap_code codes[4];
	
	assert(memory);
	assert(code==_2byte || code==_4byte);
	
	codes[0]= _begin_bs_array;
	codes[1]= count;
	codes[2]= code;
	codes[3]= _end_bs_array;
	
	_byte_swap_data(name, memory, codes, (int *) NULL, (int *) NULL);
	
	return;
}

/* ---------- code */

static void _byte_swap_data(
	char *name,
	void *data,
	byte_swap_code *codes,
	int *total_size_in_bytes,
	int *total_size_in_codes)
{
	int size_in_bytes;	
	int size_in_codes= 0;
	
	int array_size, array_index;
	
	boolean done;

	// header: _begin_bs_array
	vassert(codes[0]==_begin_bs_array,
		csprintf(temporary, "%s bs data @%p.#0 has bad start #%d", name, codes, codes[0]));
	
	// array size
	array_size= codes[1];
	vassert(array_size>=0,
		csprintf(temporary, "%s bs data @%p.#1 has invalid array size #%d", name, codes, array_size));

	size_in_bytes= 0;
	for (array_index= 0; array_index<array_size; ++array_index)
	{
		size_in_codes= 2;
		done= FALSE;
		
		while (!done)
		{
			int code= codes[size_in_codes];
			
			switch (code)
			{
				case _2byte:
					
					if (data)
					{
						word *p= (word *)((byte *)data + size_in_bytes);
						word q= *p;
						q= SWAP2(q);
						*p= q;
					}
					
					size_in_codes+= 1;
					size_in_bytes+= 2;

					break;
				
				case _4byte:
					
					if (data)
					{
						unsigned long *p= (unsigned long *)((byte *)data + size_in_bytes);
						unsigned long q= *p;
						q= SWAP4(q);
						*p= q;
					}

					size_in_codes+= 1;
					size_in_bytes+= 4;
					
					break;
				
				case _begin_bs_array:
				{
					int recursive_size_in_bytes, recursive_size_in_codes;
					
					_byte_swap_data(name, data ? ((byte *)data + size_in_bytes) : (void *) NULL, codes + size_in_codes, &recursive_size_in_bytes, &recursive_size_in_codes);
					
					size_in_codes+= recursive_size_in_codes;
					size_in_bytes+= recursive_size_in_bytes;
					
					break;
				}
				
				case _end_bs_array:
					size_in_codes+= 1;
					done= TRUE;
					break;
				
				default:
					if (code>0)
					{
						size_in_codes+= 1;
						size_in_bytes+= code;
					}
					else
					{
						vhalt(csprintf(temporary, "bs @%p.#%d has invalid code #%d", codes, size_in_codes, code));
					}
			}
		}
	}
	
	if (total_size_in_bytes) *total_size_in_bytes= size_in_bytes;
	if (total_size_in_codes) *total_size_in_codes= size_in_codes;

	return;
}
