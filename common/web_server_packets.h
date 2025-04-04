/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

#ifndef __WEB_SERVER_PACKETS_H
#define __WEB_SERVER_PACKETS_H

enum
{
	_query_motd_packet,
	_change_motd_packet,
	_new_user_packet,
	_change_password_packet,
	_create_order_packet,
	_join_order_packet,
	_leave_order_packet,
	_update_order_information_packet,
	_query_order_information_packet,
	_boot_player_from_order_packet,
	_lock_account_packet,
	_unlock_account_packet,
	_reset_account_packet,
	_reset_game_type_packet,
	_reset_order_packet,

	_web_server_response_packet = 100,
	_query_motd_response_packet,
	_change_motd_response_packet,
	_new_user_response_packet,
	_change_password_response_packet,
	_create_order_response_packet,
	_join_order_response_packet,
	_leave_order_response_packet,
	_update_order_information_response_packet,
	_query_order_information_response_packet,
	_boot_player_from_order_response_packet,
	_lock_account_response_packet,
	_unlock_account_response_packet,
	_reset_account_response_packet,
	_reset_game_type_response_packet,
	_reset_order_response_packet,
};

enum
{
	_failure,
	_success
};

struct web_server_response_packet
{
	short response_type;
	short operation_code;
	short data_length;
	short unused;
};

struct lock_account_packet
{
	long ban_duration;
};

struct reset_game_type_packet
{
	short order;
	short type;
};

short build_web_server_response_packet(
	char * buffer, 
	short response_type,
	short operation_code,
	short data_length,
	void * data);

#endif
