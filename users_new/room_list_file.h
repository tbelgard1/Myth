/*
Part of the Bungie.net Myth2 Metaserver source code
Copyright (c) 1997-2002 Bungie Studios
Refer to the file "License.txt" for details
*/

struct room_data {
	short game_type;
	short ranked_room;
	short room_identifier;
	short country_code;
	short minimum_caste;
	short maximum_caste;
	short tournament_room;
	boolean used;
	struct room_data *next;
};

struct room_data *load_room_list(void);
short game_name_to_type(char *name);
char *get_game_name_from_type(short type);
boolean save_room_list(struct room_data *rooms, char *filename);
void list_room_templates(struct room_data *rooms);
struct room_data *delete_room_template(struct room_data *rooms, short game_type, 
	short room_identifier);
	
struct room_data *add_or_update_room(struct room_data *rooms, short game_type,
	short room_identifier, short ranked_room, short country_code, short minimum_caste, 
	short maximum_caste, short tournament_room);

