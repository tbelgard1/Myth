# Part of the Bungie.net Myth2 Metaserver source code
# Copyright (c) 1997-2002 Bungie Studios
# Refer to the file "License.txt" for details

# build settings

####### compiler settings
CPP = g++
CC = gcc

# compiler flags
# -g				generate extra debugging info
# -On				optimization level
# -W				warnings
# -D DEBUG			extra debugging
# -D BN2_FULLVERSION, BN2_DEMOVERSION		 building full or demo version
# -D HARDCODE_USERD_SETTINGS	hardcode server settings to avoid using getenv()
# -D RUNNING_LOCALLY set if testing on the localhost

# Note: you will get a *ton* of warnings, don't be alarmed :O)
# Note: the *ton* of warnings have been removed by Alan Wagner :)
CFLAGS = \
		-O2 \
		-Wall \
		-D linux \
		-D BN2_FULLVERSION \
-D CYGWIN \
		-D HARDCODE_USERD_SETTINGS


####### linker settings
# Note: Linker has been altered by MythDev to allow .cpp file inclusion
# LINK = gcc
LINK = g++
LFLAGS =

####### Implicit rules

.SUFFIXES: .c .cpp

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

.cpp.o:
	$(CPP) -c $(CFLAGS) $(INCPATH) -o $@ $<
