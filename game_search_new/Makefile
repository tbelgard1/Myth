# Part of the Bungie.net Myth2 Metaserver source code
# Copyright (c) 1997-2002 Bungie Studios
# Refer to the file "License.txt" for details

####### Makefile for game_search_server

####### Compiler settings

# load from settings file
include ../build_settings.txt

INCPATH = -I../common -I../game_search_new -I../users_new -I../room_new -I../utils

LIBS		= -lm -lsqlite3


####### Target

DESTDIR = ./
TARGET	= game_search_server_new

####### Files

HEADERS =	

SOURCES = 	../sqlite_utils.c \
		game_search_server.c \
		games_list.c \
		../common/cseries.c \
		../common/network_queues.c \
		../common/metaserver_packets.c \
		../common/byte_swapping.c \
		../common/game_search_packets.c \
		../common/sl_list.c \
		../utils/environment.c

OBJECTS = 	../sqlite_utils.o \
		game_search_server.o \
		games_list.o \
		../common/cseries.o \
		../common/network_queues.o \
		../common/metaserver_packets.o \
		../common/byte_swapping.o \
		../common/game_search_packets.o \
		../common/sl_list.o \
		../utils/environment.o



####### Build rules

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	-rm -f *.o *~ core $(TARGET)

tarball:
	cd ../; tar cBf - ./game_search_new/*.[c,cpp,h] \
		./game_search_new/Makefile \
		> ./game_search_new/server.tar

####### Compile

game_search_server.o: game_search_server.c
games_list.o: games_list.c
../common/cseries.o: ../common/cseries.c
../common/network_queues.o: ../common/network_queues.c
../common/metaserver_packets.o: ../common/metaserver_packets.c
../common/byte_swapping.o: ../common/byte_swapping.c
../common/game_search_packets.o: ../common/game_search_packets.c
../common/sl_list.o: ../common/sl_list.c
../utils/environment.o: ../utils/environment.c
