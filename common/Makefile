# Part of the Bungie.net Myth2 Metaserver source code
# Copyright (c) 1997-2002 Bungie Studios
# Refer to the file "License.txt" for details

# load from settings file
include ../build_settings.txt

INCPATH = -I. -I../users_new -I../game_search -I../update -I../utils -I../room_new

LIBS		= 


all : authentication.o byte_swapping.o cseries.o metaserver_packets.o network_queues.o room_packets.o stats.o game_search_packets.o rb_tree.o web_server_packets.o sl_list.o

clean :
	echo "Removing all old object files"
	rm -f *.o
