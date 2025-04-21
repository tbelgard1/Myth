/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

boolean create_order_database(
	void);

boolean initialize_order_database(
	void);

void shutdown_order_database(
	void);

unsigned long get_order_count(
	void);

boolean get_first_order_information(
	struct bungie_net_order_datum * order);

boolean get_next_order_information(
	struct bungie_net_order_datum * order);

boolean get_order_information(
	char * order_name,
	unsigned long order_id,
	struct bungie_net_order_datum * order);

boolean update_order_information(
	char * order_name,
	unsigned long order_id,
	struct bungie_net_order_datum * order);

boolean new_order(
	struct bungie_net_order_datum * order);

void mark_order_as_unused(
	unsigned long order_id);





