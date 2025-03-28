#include "cseries.h"
#include "byte_swapping.h"
#include "metaserver_common_structs.h"
#include "authentication.h"
#include "stats.h"
#include "bungie_net_player.h"
#include "bungie_net_order.h"
#include "users.h"
#include "games.h"
#include "rank.h"
#include "metaserver_packets.h"
#include "room_packets.h"
#include "game_search_packets.h"
#include "web_server_packets.h"

static short build_empty_header(
	char * buffer,
	short type);

static short append_data_to_packet(
	char * buffer,
	void * data,
	short data_length);

short build_web_server_response_packet(
	char * buffer, 
	short response_type,
	short operation_code,
	short data_length,
	void * data)
{
	build_empty_header(buffer, _web_server_response_packet);
	append_data_to_packet(buffer, &response_type, sizeof(short));
	append_data_to_packet(buffer, &operation_code, sizeof(short));
	append_data_to_packet(buffer, &data_length, sizeof(short));
	append_data_to_packet(buffer, &operation_code, sizeof(short));
	return append_data_to_packet(buffer, data, data_length);
}

static short build_empty_header(
	char * buffer,
	short type)
{
	struct room_packet_header * header;

	header = (struct room_packet_header *)buffer;
	header->type = type;
	header->length = sizeof(struct room_packet_header);

	return header->length;
}

static short append_data_to_packet(
	char * buffer,
	void * data,
	short data_length)
{
	struct room_packet_header * header;
	char * dest;

	header = (struct room_packet_header *)buffer;
	dest = buffer + header->length;

	memcpy(dest, data, data_length);
	header->length += data_length;

	return header->length;
}
