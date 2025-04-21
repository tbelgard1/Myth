#ifndef __PLAYER_SEARCH_PACKETS_H
#define __PLAYER_SEARCH_PACKETS_H

#define	DEFAULT_PLAYER_SEARCH_PORT		7981

enum
{
	_ps_login_packet,
	_ps_add_player_packet,
	_ps_delete_player_packet,
	_ps_query_packet,
	_ps_query_response_packet
};	

enum
{
	_player_search_query,
	_buddy_query,
	_order_query
};

struct ps_login_packet
{
	long room_id;
};

struct ps_add_player_packet
{
	long player_id;
	short order;
	short unused;
	long room_id;
	struct metaserver_player_aux_data player_aux_data;
	// void * player_data ...
};

struct ps_delete_player_packet
{
	long player_id;
	long room_id;
};

struct ps_query_packet
{
	long player_id;
	long buddy_ids[8];
	short order;
	short unused_short;
	// char string [] - query
};

struct ps_query_response_header
{
	long player_id;
	short type; 
	short number_of_responses;
};

struct ps_query_response_segment
{
	long room_id;
	struct metaserver_player_aux_data player_aux_data;
	// metaserver_player_info ...
};

short build_ps_login_packet(char * buffer, long room_id);
short build_ps_add_player_packet(char * buffer, long room_id, long player_id, short order, struct metaserver_player_aux_data * player_aux_data, char * player_data);
short build_ps_delete_player_packet(char * buffer, long player_id, long room_id);
short build_ps_query_packet(char * buffer, long player_id, long * buddy_list, short order, char * search_string);
short build_ps_query_response_packet(char * buffer, short type, long user_id, struct query_response * qr);
boolean send_player_search_packet(int socket, char * buffer, short length);
boolean byte_swap_player_search_packet(char * buffer, boolean outgoing);

struct tag_P_BS_DATA
{
	byte_swap_code * 		code;
	short				length;
};

typedef struct tag_P_BS_DATA P_BS_DATA;

#endif  // __PLAYER_SEARCH_PACKETS_H
