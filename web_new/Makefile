# Added to the Bungie.net Myth2 Metaserver source code by Vishvananda Ishaya
# Copyright (c) 2002 Vishvananda Ishaya
# Refer to the file "License.txt" for details

####### Makefile for webdeamon by MythDev

####### Compiler settings

# load from settings file
include ../build_settings.txt

INCPATH = -I../common -I../users_new -I../game_search_new -I../room_new -I../utils

LIBS		= -lm -lsqlite3

####### Target

DESTDIR = ./
TARGET	= webd_new

####### Files

HEADERS =	

SOURCES =	webd_new.c \
		get_userd_host.c \
		../common/cseries.c \
		../common/network_queues.c \
		../common/metaserver_packets.c \
		../common/byte_swapping.c \
		../common/web_server_packets.c \
		../common/sl_list.c \
		../utils/environment.c

OBJECTS = ../sqlite_utils.o webd_new.o get_userd_host.o \
		../common/cseries.o \
		../common/network_queues.o \
		../common/metaserver_packets.o \
		../common/byte_swapping.o \
		../common/web_server_packets.o \
		../common/sl_list.o \
		../utils/environment.o



####### Build rules

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

clean:
	-rm -f *.o *~ core $(TARGET)

tarball:
	cd ../; tar cBf - ./web_new/*.[c,cpp,h] \
		./web_new/Makefile \
		> ./web_new/web_daemon.tar

####### Compile

webd_new.o: webd_new.c
../common/cseries.o: ../common/cseries.c
../common/network_queues.o: ../common/network_queues.c
../common/metaserver_packets.o: ../common/metaserver_packets.c
../common/byte_swapping.o: ../common/byte_swapping.c
../common/web_server_packets.o: ../common/web_server_packets.c
../common/sl_list.o: ../common/sl_list.c
../utils/environment.o: ../utils/environment.c
