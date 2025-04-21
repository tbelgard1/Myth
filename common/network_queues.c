/*
	network_queues.c
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

/*
	SERVER:
		data comes in on client, copied to clientÕs internal queue.
		at processing time, each clientÕs queue, in succession, is copied to the outgoing buffer.
		the outgoing buffer is then sent to each player (and their read_index is kept track of on a per-player basis)
		if the outgoing packet is not destined for a given player, their read index is just incremented and skipped...
	CLIENT:
		data comes in, copied into the internal queue.
		outgoing data is copied into the outgoing queue, and when time arrives, it is sent. (this could be a 
			linear command queue, like the monsters, since we will always have all the data at once)
*/

/*
	uber.h, network.h are only used for sizeof(struct net_message_header) and header->messageLen
*/

#include "cseries.h"
#include <string.h>

#include "authentication.h"
#include "metaserver_codes.h"
#include "metaserver_common_structs.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "network_queues.h"
#include "byte_swapping.h"

enum {
	SLIP_END= 0xc0,
	SLIP_ESC= 0xdb,
	SLIP_ESC_END= 0xdc,
	SLIP_ESC_ESC= 0xdd
};

#define CIRCULAR_BUFFER_SIZE(b) (((b)->read_index<=(b)->write_index) ? ((b)->write_index-(b)->read_index) : ((b)->size-(b)->read_index+(b)->write_index))
#define FREE_CIRCULAR_BUFFER_SIZE(q) (((q)->read_index<=(q)->write_index) ? ((q)->size-1-(q)->write_index+(q)->read_index) : ((q)->read_index-1-(q)->write_index))

static boolean valid_queue(struct circular_queue *queue);

short copy_data_from_circular_queue(
	char * buffer,
	short length,
	struct circular_queue * queue)
{
	if (CIRCULAR_BUFFER_SIZE(queue) < length)
		length = CIRCULAR_BUFFER_SIZE(queue);

	if (queue->read_index <= queue->write_index)
	{
		memcpy(buffer, queue->buffer + queue->read_index, length);
		queue->read_index += length;
	}
	else
	{
		int n = queue->size - queue->read_index;
		if (n >= length)
		{
			memcpy(buffer, (queue->buffer + queue->read_index), length);
			queue->read_index += length;
		}
		else
		{
			memcpy(buffer, (queue->buffer + queue->read_index), n);
			memcpy(buffer + n, queue->buffer, length - n);
			queue->read_index = length - n;
		}
	}

	return length;
}

// this should be highly optimized (copy by longs, then shorts, then bytes)
boolean copy_data_into_circular_queue(
	char *data,
	short length,
	struct circular_queue *queue)
{
	boolean success= FALSE;

#ifdef INSTRUMENT_QUEUES
	if(length>queue->largest_write) queue->largest_write= length;
	if(length<queue->smallest_write) queue->smallest_write= length;
	queue->number_of_bytes_sent+= length;
#endif

	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));
	if(FREE_CIRCULAR_BUFFER_SIZE(queue)>=length)
	{
		char *dest= queue->buffer+queue->write_index;

		while(length--)
		{	
			*dest++= *data++;
			if(++queue->write_index>=queue->size) 
			{
				queue->write_index= 0;
				dest= queue->buffer;
			}
		}
		success= TRUE;
	}
	
	return success;
}

boolean copy_slip_data_into_circular_queue(
	struct circular_queue *queue, 
	unsigned char *data, 
	short length)
{
	boolean success= FALSE;
	short actual_length;
	
	{
		unsigned char *p= data;
		short size= length;
		
		actual_length= 1; // first END (0xc0)
		while(size--)
		{
			switch(*p)
			{
				case 0xc0:
					actual_length+= 1; // 0xdb 0xdc
					break;
					
				case 0xdb:
					actual_length+= 1; // 0xdb 0xdd
					break;
			}
			actual_length+= 1;
			p++;
		}
		actual_length+= 1; // last end (0xc0)
	}

#ifdef INSTRUMENT_QUEUES
	if(actual_length>queue->largest_write) queue->largest_write= actual_length;
	if(actual_length<queue->smallest_write) queue->smallest_write= actual_length;
	queue->number_of_bytes_sent+= actual_length;
#endif

	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));
	if(FREE_CIRCULAR_BUFFER_SIZE(queue)>=actual_length)
	{
		char *dest= queue->buffer+queue->write_index;

		// first end
		*dest += SLIP_END;
		if(++queue->write_index>=queue->size) 
		{
			queue->write_index= 0;
			dest= queue->buffer;
		}
		
		// body
		while(length--)
		{	
			switch(*data)
			{
				case SLIP_END:
					*dest += SLIP_ESC;
					if(++queue->write_index>=queue->size) 
					{
						queue->write_index= 0;
						dest= queue->buffer;
					}
					*dest += SLIP_ESC_END;
					break;
					
				case SLIP_ESC:
					*dest += SLIP_ESC;
					if(++queue->write_index>=queue->size) 
					{
						queue->write_index= 0;
						dest= queue->buffer;
					}
					*dest += SLIP_ESC_ESC;
					break;
					
				default:
					// normal case..
					*dest += *data++;
					break;
			}
			
			// and then increment...
			if(++queue->write_index>=queue->size) 
			{
				queue->write_index= 0;
				dest= queue->buffer;
			}
		}

		// last end
		*dest += SLIP_END;
		if(++queue->write_index>=queue->size) 
		{
			queue->write_index= 0;
			dest= queue->buffer;
		}

		success= TRUE;
	}
	
	return success;
}

boolean allocate_circular_queue(
	char *name,
	struct circular_queue *queue, 
	long size)
{
	boolean success= FALSE;

	memset(queue, 0, sizeof(struct circular_queue));
	strcpy(queue->name, name);
	queue->size= size;
	queue->buffer= malloc(queue->size);
#ifdef INSTRUMENT_QUEUES
	queue->largest_write= LONG_MIN;
	queue->smallest_write= LONG_MAX;
	queue->number_of_bytes_sent= 0;
	queue->number_of_packets= 0;
	queue->smallest_packet= LONG_MAX;
	queue->largest_packet= LONG_MIN;
#endif
	if(queue->buffer)
	{
		memset(queue->buffer, 0xfe, queue->size );
		success= TRUE;
	}
	
	return success;
}

void free_circular_queue(
	struct circular_queue *queue)
{
	assert(queue);
	if(queue->buffer)
	{
		// instrumentation.
		dump_queue_statistics(queue);
		
		free(queue->buffer);
		queue->buffer= NULL;
	}
	
	return;
}

boolean copy_queue_into_queue(
	struct circular_queue *source, 
	struct circular_queue *destination)
{
	boolean success= FALSE;
	short length= CIRCULAR_BUFFER_SIZE(source);
	
	vassert(valid_queue(source), csprintf(temporary, "queue %s is invalid!", source->name));
	vassert(valid_queue(destination), csprintf(temporary, "queue %s is invalid!", destination->name));

#ifdef INSTRUMENT_QUEUES
	if(length>destination->largest_write) destination->largest_write= length;
	if(length<destination->smallest_write) destination->smallest_write= length;
	destination->number_of_bytes_sent+= length;
#endif
	
	if(FREE_CIRCULAR_BUFFER_SIZE(destination)>=length)
	{
		char *dest= destination->buffer+destination->write_index;
		char *src= source->buffer+source->read_index;

		while(length-->0)
		{
			*dest++= *src++;
			if(++destination->write_index>=destination->size)
			{
				dest= destination->buffer;
				destination->write_index= 0;
			}
			if(++source->read_index>=source->size)
			{
				src= source->buffer;
				source->read_index= 0;
			}
		}
		vassert(dest-destination->buffer==destination->write_index,
			csprintf(temporary, "dest: 0x%x dest->buffer: 0x%x write index: %d", dest,destination->buffer, destination->write_index));
		vassert(src-source->buffer==source->read_index,
			csprintf(temporary, "src: 0x%x src->buffer: 0x%x read index: %d",src,source->buffer,source->read_index));
		success= TRUE;
	}

	return success;	
}

long circular_buffer_size(
	struct circular_queue *queue)
{
	assert(queue);
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));
	
	return CIRCULAR_BUFFER_SIZE(queue);
}

long circular_buffer_free_size(
	struct circular_queue *queue)
{
	assert(queue);
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));
	
	return FREE_CIRCULAR_BUFFER_SIZE(queue);
}

long circular_buffer_linear_write_size(
	struct circular_queue *queue)
{
	long write_size;
	
	assert(queue);
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));
	
	write_size= (queue->write_index>=queue->read_index) ? (queue->write_index-queue->read_index) : (queue->size-queue->read_index);
	
	return write_size;	
}

void dump_queue(
	struct circular_queue *queue)
{
	// ALAN Begin: remove compiler warnings (wrong format for args)
//	printf("Queue %s at 0x%x;g", queue->name, queue);
//	printf("Read at %d Write: %d Size: %d;g", queue->read_index, queue->write_index, queue->size);
//	printf("Buffer: 0x%x", queue->buffer);
	printf("Queue %s\n", queue->name);
	printf("Read at %ld Write: %ld Size: %ld;g\n", queue->read_index, queue->write_index, queue->size);
	printf("Buffer: %s\n", queue->buffer);
	// ALAN End
	
	return;
}

boolean get_next_packet_from_queue(
	struct circular_queue *queue, 
	struct packet_header *packet, 
	long maximum_length,
	boolean byteswap,
	boolean * disconnect)
{
	boolean got_a_packet= FALSE;

	assert(queue);
	assert(packet);
	assert(maximum_length>=sizeof(struct packet_header));
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));

//printf("Read: %d Write: %d Buffer Size: %d\n", queue->read_index, queue->write_index, queue->size);
	if(CIRCULAR_BUFFER_SIZE(queue)>=sizeof(struct packet_header))
	{
		long initial_read_index= queue->read_index;
		byte *dest= (byte *) packet;
		long length;

		// read the header (it could straddle)
		length= sizeof(struct packet_header);
		while(length-->0)
		{
			*dest++= queue->buffer[initial_read_index];
			if(++initial_read_index>=queue->size) initial_read_index= 0;
		}

		// NOTE THAT WE HAVE NO ERROR CHECKING TO AVOID SPURIOUS PACKETS! THIS BETTER BE RIGHT!
		// calculate the length.			
		length= packet->length;
		if(byteswap) // on little_endian machines, the status buffer is not byteswapped.
		{
			length= SWAP4(length);
		}

		if(length>maximum_length)
		{
			// ALAN Begin: remove compiler warnings (wrong format for args)
			printf("q: %s got unusually large packet: length: %ld max: %ld byteswap: %d\n",
 				queue->name, length, maximum_length, byteswap);
			// ALAN End

 			dump_queue(queue);
			*disconnect = TRUE;
			return FALSE;
 		}

		/* Go with this one.. */
		if(CIRCULAR_BUFFER_SIZE(queue)>=length) // not length-sizeof(struct net_message_header), because we have to have enough for all of it.
		{
			char *source;
			short index;
		
			/* Okay, this is totally valid- copy into the linear packet */
			dest= (char *) packet;
			source= queue->buffer+queue->read_index;
			for(index= 0; index<length; ++index)
			{
				*dest++= *source++;
				if(++queue->read_index>=queue->size)
				{
					queue->read_index= 0;
					source= queue->buffer;
				}
			}

			got_a_packet= TRUE;
#ifdef INSTRUMENT_QUEUES
			queue->number_of_packets+= 1;
			if(length<queue->smallest_packet) queue->smallest_packet= length;
			if(length>queue->largest_packet) queue->largest_packet= length;
#endif
		} 
	}

	return got_a_packet;
}

boolean get_next_room_packet_from_queue(
	struct circular_queue *queue, 
	struct room_packet_header *packet, 
	long maximum_length,
	boolean byteswap,
	boolean * disconnect)
{
	boolean got_a_packet= FALSE;

	assert(queue);
	assert(packet);
	assert(maximum_length>=sizeof(struct room_packet_header));
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));

 	if(CIRCULAR_BUFFER_SIZE(queue)>=sizeof(struct room_packet_header))
	{
		long initial_read_index= queue->read_index;
		byte *dest= (byte *) packet;
		short length;

		// read the header (it could straddle)
		length= sizeof(struct room_packet_header);
		while(length-->0)
		{
			*dest++= queue->buffer[initial_read_index];
			if(++initial_read_index>=queue->size) initial_read_index= 0;
		}

		// calculate the length.			
		length= packet->length;
		if(byteswap) // on little_endian machines, the status buffer is not byteswapped.
		{
			length= SWAP2(length);
		}

		if(length>maximum_length)
		{
			// ALAN Begin: remove compiler warnings (wrong format for args)
			printf("q: %s got unusually large packet: length: %d max: %ld byteswap: %d\n",
 				queue->name, length, maximum_length, byteswap);
			// ALAN End

 			dump_queue(queue);
			*disconnect = TRUE;
			queue->read_index= queue->write_index= 0;
 			return FALSE;
 		}
	
		if (!length)
		{
			got_a_packet= FALSE;
			queue->read_index= queue->write_index= 0;
		}
		else if(CIRCULAR_BUFFER_SIZE(queue)>=length) // not length-sizeof(struct net_message_header), because we have to have enough for all of it.
		{
			char *source;
			short index;
		
			/* Okay, this is totally valid- copy into the linear packet */
			dest= (char *) packet;
			source= queue->buffer+queue->read_index;
			for(index= 0; index<length; ++index)
			{
				*dest++= *source++;
				if(++queue->read_index>=queue->size)
				{
					queue->read_index= 0;
					source= queue->buffer;
				}
			}

			got_a_packet= TRUE;
#ifdef INSTRUMENT_QUEUES
			queue->number_of_packets+= 1;
			if(length<queue->smallest_packet) queue->smallest_packet= length;
			if(length>queue->largest_packet) queue->largest_packet= length;
#endif
		} 
	}

	return got_a_packet;
}

#if 0
boolean get_next_packet_from_queue(
	struct circular_queue *queue, 
	struct net_message_header *packet, 
	short maximum_length,
	boolean byteswap)
{
	boolean got_a_packet= FALSE;

	assert(queue);
	assert(packet);
	assert(maximum_length>=sizeof(struct net_message_header));
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));

 	if(CIRCULAR_BUFFER_SIZE(queue)>=sizeof(struct net_message_header))
	{
		long initial_read_index= queue->read_index;
		byte *dest= (byte *) packet;
		short length;

		// read the header (it could straddle)
		length= sizeof(struct net_message_header);
		while(length-->0)
		{
			*dest++= queue->buffer[initial_read_index];
			if(++initial_read_index>=queue->size) initial_read_index= 0;
		}

		// calculate the length.			
		length= packet->messageLen;
		if(byteswap) // on little_endian machines, the status buffer is not byteswapped.
		{
			length= SWAP2(length);
		}

		if(length>maximum_length)
		{
			printf("q: %s got unusually large packet: length: %d max: %d byteswap: %d",
 				queue->name, length, maximum_length, byteswap));
 			dump_queue(queue);
 			halt();
 		}
	
		/* Go with this one.. */
		if(CIRCULAR_BUFFER_SIZE(queue)>=length) // not length-sizeof(struct net_message_header), because we have to have enough for all of it.
		{
			char *source;
			short index;
		
			/* Okay, this is totally valid- copy into the linear packet */
			dest= (byte *) packet;
			source= queue->buffer+queue->read_index;
			for(index= 0; index<length; ++index)
			{
				*dest++= *source++;
				if(++queue->read_index>=queue->size)
				{
					queue->read_index= 0;
					source= queue->buffer;
				}
			}

			got_a_packet= TRUE;
#ifdef INSTRUMENT_QUEUES
			queue->number_of_packets+= 1;
			if(length<queue->smallest_packet) queue->smallest_packet= length;
			if(length>queue->largest_packet) queue->largest_packet= length;
#endif
		} 
	}

	return got_a_packet;
}
#endif

boolean parse_ftp_data_from_circular_queue(
	struct circular_queue *queue, 
	short *code, 
	char *message,
	boolean *more_to_come)
{
	boolean got_a_packet= FALSE;
	static short last_code= NONE;

	assert(queue);
	assert(code && message);
	vassert(valid_queue(queue), csprintf(temporary, "queue %s is invalid!", queue->name));

	if(CIRCULAR_BUFFER_SIZE(queue))
	{
		long initial_read_index= queue->read_index;
		char *dest= message;

		// read the header (it could straddle)
		do {
			char p= queue->buffer[initial_read_index];
			*dest++= p;
			
			if(p=='\n')
			{
				got_a_packet= TRUE;
			}
		
			if(++initial_read_index>=queue->size) initial_read_index= 0;
		} while(!got_a_packet && initial_read_index != queue->write_index);

		if(got_a_packet)
		{
			// null terminate
			*dest++= '\0';
			*code= atoi(message);

			// 123-, ignore everything until the next 123
			if(last_code!=NONE)
			{
				if(*code==last_code)
				{
					last_code= NONE;
					*more_to_come= FALSE;
				} else {
					*more_to_come= TRUE;
				}
			} else {
				// 123- more to come...
				if(message[3]=='-') 
				{
					last_code= *code;
					*more_to_come= TRUE;
				} else {
					last_code= NONE;
					*more_to_come= FALSE;
				}
			}
			queue->read_index= initial_read_index; // go there
		}		
	}


	return got_a_packet;
}

enum {
	_state_reading_first_end,
	_state_reading_escaped_character,
	_state_reading_data,
	_state_done,
	NUMBER_OF_SLIP_STATES
};

boolean get_next_slip_packet_from_queue(
	struct circular_queue *queue, 
	unsigned char *data, 
	short *ioLength)
{
	boolean valid_packet= FALSE;
	short length= 0;
	
	if(CIRCULAR_BUFFER_SIZE(queue))
	{
		unsigned char *source;
		unsigned char *dest= data;
	
		// gotta try for it...
		// read until the first end character, because if it isn't the first, there's garbage
		source= queue->buffer+queue->read_index;
		while(*source != SLIP_END)
		{
			source++;
			if(++queue->read_index>=queue->size)
			{
				queue->read_index= 0;
				source= queue->buffer;
			}
		}
		
		// if there still is anything in the buffer...
		if(CIRCULAR_BUFFER_SIZE(queue)>0)
		{
			long temp_read_index= queue->read_index; // in case the entire thing hasn't arrived yet.
			short state= _state_reading_first_end;
			
			source= queue->buffer+temp_read_index;
			do {
				switch(*source)
				{
					case SLIP_END:
						if(state==_state_reading_first_end)
						{
							state= _state_reading_data;
						} else {
							state= _state_done;
							valid_packet= TRUE;
						}
						break;
						
					case SLIP_ESC:
						state= _state_reading_escaped_character;
						break;
						
					case SLIP_ESC_END:
						if(state==_state_reading_escaped_character)
						{
							*dest++= SLIP_END;
							length++;
							state= _state_reading_data;
						} else {
							// not escaped..
							*dest++= SLIP_ESC_END;
							length++;
						}
						break;
						
					case SLIP_ESC_ESC:
						if(state==_state_reading_escaped_character)
						{
							*dest++= SLIP_ESC;
							length++;
							state= _state_reading_data;
						} else {
							// not escaped..
							*dest++= SLIP_ESC_ESC;
							length++;
						}
						break;
						
					default:
						assert(state==_state_reading_data);
						*dest++= *source;
						length++;
						break;
				}

				// increment the source
				source++;
				if(++temp_read_index>=queue->size)
				{
					temp_read_index= 0;
					source= queue->buffer;

					// if not a complete packet...
					if(temp_read_index==queue->write_index) state= _state_done;
				}
			} while(state != _state_done);
			
			// we successfully read one out, get rid of it!
			if(valid_packet)
			{
				queue->read_index= temp_read_index;
			}
		}
	}
	
	if(valid_packet)
	{
		*ioLength= length;
	}
	
	return valid_packet;
}

static boolean valid_queue(
	struct circular_queue *queue)
{
	boolean valid= FALSE;
	
	if(queue && queue->buffer)
	{
		if(queue->read_index>=0 && queue->read_index<queue->size)
		{
			if(queue->write_index>=0 && queue->write_index<queue->size)
			{
				valid= TRUE;
			}
		}
	}

	if(!valid)
	{
		dump_queue(queue);
	}
	
	return valid;
}

#ifdef INSTRUMENT_QUEUES
void dump_queue_statistics(
	struct circular_queue *queue)
{
	printf("Queue statistics for %s", queue->name));
	printf("Write: %d bytes (%d, %d)", queue->number_of_bytes_sent, queue->smallest_write, queue->largest_write));
	printf("Read: %d packets (%d, %d)", queue->number_of_packets, queue->smallest_packet, queue->largest_packet));

	return;
}
#endif
