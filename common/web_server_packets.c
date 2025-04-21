
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

// ALAN Begin: added headers
#include <string.h>
// ALAN End

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


// _MYTHDEV Begin: 

// VISH: modified this file to have a bunch of packet builders
short build_query_motd_packet(
	char * buffer,
	char * args[])
{
	return build_empty_header(buffer, _query_motd_packet);
}

short build_change_motd_packet(
	char * buffer,
	char * args[])
{
	char * new_motd = args[0];
	build_empty_header(buffer, _change_motd_packet);
	return append_data_to_packet(buffer, new_motd, strlen(new_motd) + 1);
}

short build_new_user_packet(
	char * buffer,
	char * args[])
{
	char * login = args[0];
	char * password = args[1];
	build_empty_header(buffer, _new_user_packet);
	append_data_to_packet(buffer, login, strlen(login) + 1);
	return append_data_to_packet(buffer, password, strlen(password) + 1);
}

short build_change_password_packet(
	char * buffer,
	char * args[])
{
	char * login = args[0];
	char * new_password = args[1];
	build_empty_header(buffer, _change_password_packet);
	append_data_to_packet(buffer, login, strlen(login) + 1);
	return append_data_to_packet(buffer, new_password, strlen(new_password) + 1);
}

short build_create_order_packet(
	char * buffer,
	char * args[])
{
	char * order_name = args[0];
	char * maintenance_password = args[1];
	char * member_password = args[2];
	char * url = args[3];
	char * contact_email = args[4];
	char * motto = args[5];
	build_empty_header(buffer, _create_order_packet);
	append_data_to_packet(buffer, order_name, strlen(order_name) + 1);
	append_data_to_packet(buffer, maintenance_password, strlen(maintenance_password) + 1);
	append_data_to_packet(buffer, member_password, strlen(member_password) + 1);
	append_data_to_packet(buffer, url, strlen(url) + 1);
	append_data_to_packet(buffer, contact_email, strlen(contact_email) + 1);
	return append_data_to_packet(buffer, motto, strlen(motto) + 1);
}

short build_join_order_packet(
	char * buffer,
	char * args[])
{
	char * order_name = args[0];
	char * member_password = args[1];
	char * login = args[2];
	char * password = args[3];
	build_empty_header(buffer, _join_order_packet);
	append_data_to_packet(buffer, order_name, strlen(order_name) + 1);
	append_data_to_packet(buffer, member_password, strlen(member_password) + 1);
	append_data_to_packet(buffer, login, strlen(login) + 1);
	return append_data_to_packet(buffer, password, strlen(password) + 1);
}

short build_leave_order_packet(
	char * buffer,
	char * args[])
{
	char * login = args[0];
	char * password = args[1];
	build_empty_header(buffer, _leave_order_packet);
	append_data_to_packet(buffer, login, strlen(login) + 1);
	return append_data_to_packet(buffer, password, strlen(password) + 1);
}

short build_update_order_information_packet(
	char * buffer,
	char * args[])
{
	char * order_name = args[0];
	char * maintenance_password = args[1];
	char * member_password = args[2];
	char * url = args[3];
	char * contact_email = args[4];
	char * motto = args[5];
	build_empty_header(buffer, _update_order_information_packet);
	append_data_to_packet(buffer, order_name, strlen(order_name) + 1);
	append_data_to_packet(buffer, maintenance_password, strlen(maintenance_password) + 1);
	append_data_to_packet(buffer, member_password, strlen(member_password) + 1);
	append_data_to_packet(buffer, url, strlen(url) + 1);
	append_data_to_packet(buffer, contact_email, strlen(contact_email) + 1);
	return append_data_to_packet(buffer, motto, strlen(motto) + 1);
}

short build_query_order_information_packet(
	char * buffer,
	char * args[])
{
	char * order_name = args[0];
	build_empty_header(buffer, _query_order_information_packet);
	return append_data_to_packet(buffer, order_name, strlen(order_name) + 1);
}

short build_boot_player_from_order_packet(
	char * buffer,
	char * args[])
{
	char * order_name = args[0];
	char * maintenance_password = args[1];
	char * login = args[2];
	build_empty_header(buffer, _boot_player_from_order_packet);
	append_data_to_packet(buffer, order_name, strlen(order_name) + 1);
	append_data_to_packet(buffer, maintenance_password, strlen(maintenance_password) + 1);
	return append_data_to_packet(buffer, login, strlen(login) + 1);
}

short build_lock_account_packet(
	char * buffer,
	char * args[])
{
	long ban_duration = atoi(args[0]);
	char * login = args[1];
	build_empty_header(buffer, _lock_account_packet);
	append_data_to_packet(buffer, &ban_duration, sizeof(ban_duration));
	return append_data_to_packet(buffer, login, strlen(login) + 1);
}

short build_unlock_account_packet(
	char * buffer,
	char * args[])
{
	char * login = args[0];
	build_empty_header(buffer, _unlock_account_packet);
	return append_data_to_packet(buffer, login, strlen(login) + 1);
}

short build_reset_account_packet(
	char * buffer,
	char * args[])
{
	char * login = args[0];
	build_empty_header(buffer, _reset_account_packet);
	return append_data_to_packet(buffer, login, strlen(login) + 1);
}

short build_reset_game_type_packet(
	char * buffer,
	char * args[])
{
	short order = atoi(args[0]);
	short game_type = atoi(args[1]);
	build_empty_header(buffer, _reset_game_type_packet);
	append_data_to_packet(buffer, &order, sizeof(order));
	return append_data_to_packet(buffer, &game_type, sizeof(game_type));
}

short build_reset_order_packet(
	char * buffer,
	char * args[])
{
	char * order_name = args[0];
	build_empty_header(buffer, _reset_order_packet);
	return append_data_to_packet(buffer, order_name, strlen(order_name) + 1);
}

// _MYTHDEV End



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
