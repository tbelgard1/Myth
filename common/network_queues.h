/*
	network_queues.h
*/

#ifndef __NETWORK_QUEUES__
#define __NETWORK_QUEUES__

/*
	SERVER:
		data comes in on client, copied to client's internal queue.
		at processing time, each client's queue, in succession, is copied to the outgoing buffer.
		the outgoing buffer is then sent to each player (and their read_index is kept track of on a per-player basis)
		if the outgoing packet is not destined for a given player, their read index is just incremented and skipped...
	CLIENT:
		data comes in, copied into the internal queue.
		outgoing data is copied into the outgoing queue, and when time arrives, it is sent. (this could be a 
			linear command queue, like the monsters, since we will always have all the data at once)
*/

// #define INSTRUMENT_QUEUES

#include "cseries.h"

/* ------- circular queue functions */
#ifdef INSTRUMENT_QUEUES
struct circular_queue 
{
	char name[32];
	long size;
	volatile long read_index;
	volatile long write_index;
	char *buffer;

	long largest_write;
	long smallest_write;
	long number_of_bytes_sent;
	long number_of_packets;
	long smallest_packet;
	long largest_packet;
};

void dump_queue_statistics(struct circular_queue *queue);

#else
struct circular_queue 
{
	char name[32];
	long size;
	volatile long read_index;
	volatile long write_index;
	char *buffer;
};

#define dump_queue_statistics(x)
#endif

boolean copy_data_into_circular_queue(char *data, short length, struct circular_queue *queue);

boolean allocate_circular_queue(char *name, struct circular_queue *queue, long size);
void free_circular_queue(struct circular_queue *queue);

//boolean get_next_packet_from_queue(struct circular_queue *queue, struct net_message_header *packet, 
//	long maximum_length, boolean byteswap);
boolean parse_ftp_data_from_circular_queue(struct circular_queue *queue, short *code, 
	char *message, boolean *more_to_come);

boolean copy_slip_data_into_circular_queue(struct circular_queue *queue, 
	unsigned char *data, short length);
boolean get_next_slip_packet_from_queue(struct circular_queue *queue, unsigned char *data, 
	short *ioLength);
	
boolean get_next_packet_from_queue(struct circular_queue *queue, struct packet_header *packet, 
	long maximum_length, boolean byteswap, boolean * disconn_from_error);
boolean get_next_room_packet_from_queue(struct circular_queue *queue, 
	struct room_packet_header *packet, long maximum_length, boolean byteswap, boolean * disconn_from_error);

boolean copy_queue_into_queue(struct circular_queue *source, struct circular_queue *destination);
short copy_data_from_circular_queue(char * buffer, short length, struct circular_queue * queue);

// these are for instrumentation.  don't use generally...
long circular_buffer_size(struct circular_queue *queue);
long circular_buffer_free_size(struct circular_queue *queue);
long circular_buffer_linear_write_size(struct circular_queue *queue);

void dump_queue(struct circular_queue *queue);

#define copy_client_data_into_outgoing_buffer(game, data, length) copy_data_into_circular_queue(data, length, &game->outgoing_buffer)
#define copy_client_data_into_incoming_buffer(game, data, length) copy_data_into_circular_queue(data, length, &game->incoming_buffer)

#endif // __NETWORK_QUEUES__
