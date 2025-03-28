# Part of the Bungie.net Myth2 Metaserver source code
# Copyright (c) 1997-2002 Bungie Studios
# Refer to the file "License.txt" for details

####### Makefile for new userd server

####### Compiler settings

# load from settings file
include ../build_settings.txt

INCPATH = -I./ -I../common -I../utils -I../room_new 

LIBS		= -lm

####### Target

DESTDIR = ./
TARGET	= userd_new

####### Files

HEADERS =	

SOURCES = 	game_evaluator.c \
		main.c \
		orders.c \
		rank.c \
		room_list_file.c \
		users.c \
		../common/stats.c		\
		../common/room_packets.c	\
		../common/byte_swapping.c	\
		../common/cseries.c	\
		../common/metaserver_packets.c	\
		../common/authentication.c	\
		../common/network_queues.c	\
		../common/web_server_packets.c \
		../utils/environment.c	\
		../common/rb_tree.c	\
		../common/sl_list.c

OBJECTS = 	game_evaluator.o \
		main.o \
		orders.o \
		rank.o \
		room_list_file.o \
		users.o \
		../common/stats.o		\
		../common/room_packets.o	\
		../common/byte_swapping.o	\
		../common/cseries.o	\
		../common/metaserver_packets.o	\
		../common/authentication.o	\
		../common/network_queues.o	\
		../common/web_server_packets.o \
		../utils/environment.o	\
		../common/rb_tree.o	\
		../common/sl_list.o


####### Build rules

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	-rm -f *.o *~ core $(TARGET)

####### Compile

server.o : server.c
client.o : client.c
connect_nonblock.o : connect_nonblock.c
errmsg.o : errmsg.c
game_evaluator.o : game_evaluator.c
main.o : main.c
orders.o : orders.c
rank.o : rank.c
room_list_file.o : room_list_file.c
users.o : users.c
../common/stats.o : ../common/stats.c
../common/room_packets.o : ../common/room_packets.c
../common/byte_swapping.o : ../common/byte_swapping.c
../common/cseries.o: ../common/cseries.c
../common/metaserver_packets.o	: ../common/metaserver_packets.c
../common/authentication.o	: ../common/authentication.c
../common/network_queues.o : ../common/network_queues.c
../common/web_server_packets.o : ../common/web_server_packets.c
../utils/environment.o	: ../utils/environment.c
../common/rb_tree.o : ../common/rb_tree.c
../common/sl_list.o : ../common/sl_list.c

