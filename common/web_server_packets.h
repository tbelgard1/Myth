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




// _MYTHDEV Begin: 

// VISH: modified this file to have a bunch of packet builders
short build_query_motd_packet(
	char * buffer,
	char * args[]);

short build_change_motd_packet(
	char * buffer,
	char * args[]);

short build_new_user_packet(
	char * buffer,
	char * args[]);

short build_change_password_packet(
	char * buffer,
	char * args[]);

short build_create_order_packet(
	char * buffer,
	char * args[]);

short build_join_order_packet(
	char * buffer,
	char * args[]);

short build_leave_order_packet(
	char * buffer,
	char * args[]);

short build_update_order_information_packet(
	char * buffer,
	char * args[]);

short build_query_order_information_packet(
	char * buffer,
	char * args[]);

short build_boot_player_from_order_packet(
	char * buffer,
	char * args[]);

short build_lock_account_packet(
	char * buffer,
	char * args[]);
	
short build_unlock_account_packet(
	char * buffer,
	char * args[]);

short build_reset_account_packet(
	char * buffer,
	char * args[]);

short build_reset_game_type_packet(
	char * buffer,
	char * args[]);

short build_reset_order_packet(
	char * buffer,
	char * args[]);

// _MYTHDEV End

#endif
