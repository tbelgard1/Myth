# Part of the Bungie.net Myth2 Metaserver source code
# Copyright (c) 1997-2002 Bungie Studios
# Refer to the file "License.txt" for details

# Makefile for bnetII metaserver source

# Makefile modified by MythDev

# Rules

all: game_search_ roomd userd webd install
	@echo
	@echo bnetII metaserver -MythDev build- has been successfully installed!

game_search_:
	cd ./game_search_new; $(MAKE)

roomd:
	cd ./room_new; $(MAKE)

userd:
	cd ./users_new; $(MAKE)

webd:
	cd ./web_new; $(MAKE)

install:
	cp ./game_search_new/game_search_server_new ./bin/;
	cp ./users_new/userd_new ./bin/;
	cp ./room_new/roomd_new ./bin/;
	cp ./web_new/webd_new ./bin/;
	chmod +sx ./bin/*;

clean:
	cd ./game_search_new; $(MAKE) clean
	cd ./room_new; $(MAKE) clean
	cd ./users_new; $(MAKE) clean
	cd ./web_new; $(MAKE) clean
	cd ./common; $(MAKE) clean

archive: clean
	cd ../; tar cBf - metaserver2open/ > metaserver2open.tar; gzip metaserver2open.tar
