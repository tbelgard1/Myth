# Part of the Bungie.net Myth2 Metaserver source code
# Copyright (c) 1997-2002 Bungie Studios
# Refer to the file "License.txt" for details

####### Makefile for room_server

####### Makefile modified by MythDev

####### Compiler settings

# load from settings file
include ../build_settings.txt

INCPATH = -I../common -I../users_new -I../game_search_new -I../room_new -I../utils

LIBS		= -lm -lsqlite3

####### Target

DESTDIR = ./
TARGET	= roomd_new

####### Files

HEADERS =	

SOURCES =	games.c \
		remote_commands.c \
		roomd_new.c \
		server_code.c \
		games_log.c \
		../common/stats.c		\
		../common/room_packets.c	\
		../common/byte_swapping.c	\
		../common/cseries.c	\
		../common/metaserver_packets.c	\
		../common/authentication.c	\
		../common/network_queues.c \
		../common/game_search_packets.c \
		../common/DiffieHellman.cpp \
		../common/encodePackets.cpp \
		../utils/environment.c \
		../sqlite_utils.c

OBJECTS = ../sqlite_utils.o \
		remote_commands.o \
		server_code.o \
		games_log.o \
		roomd_new.o \
		games.o \
		../common/stats.o		\
		../common/room_packets.o	\
		../common/byte_swapping.o	\
		../common/cseries.o	\
		../common/metaserver_packets.o	\
		../common/authentication.o	\
		../common/network_queues.o \
		../common/game_search_packets.o \
		../common/DiffieHellman.o \
		../common/encodePackets.o \
		../utils/environment.o



####### Build rules

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	-rm -f *.o *~ core $(TARGET)

tarball:
	cd ../; tar cBf - ./room_new/*.[c,cpp,h] \
		./room_new/Makefile \
		> ./room_new/room_server.tar

####### Compile

games.o: games.c
remote_commands.o: remote_commands.c
roomd_new.o: roomd_new.c
server_code.o: server_code.c
games_log.o: games_log.c
server_struct.o: server_struct.c
client_struct.o: client_struct.c
../common/stats.o: ../common/stats.c
../common/room_packets.o: ../common/room_packets.c
../common/byte_swapping.o: ../common/byte_swapping.c
../common/cseries.o: ../common/cseries.c
../common/metaserver_packets.o: ../common/metaserver_packets.c
../common/authentication.o: ../common/authentication.c
../common/network_queues.o: ../common/network_queues.c
../common/game_search_packets.o: ../common/game_search_packets.c
../common/DiffieHellman.o: ../common/DiffieHellman.cpp
../common/encodePackets.o: ../common/encodePackets.cpp
../utils/environment.o: ../utils/environment.c
	$(CC) -I../common -c ../utils/environment.c -o ../utils/environment.o
