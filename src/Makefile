#
#  flacplay.dll makefile for gnu MAKE and g++
#

# build environment
#-----------------------
#  gcc -v
# 
# Reading specs from J:/usr/gcc/3.3.5-csd3/lib/gcc-lib/i386-pc-os2-emx/3.3.5/specs
# Configured with: D:/CODING/LIBC/SVN/BRANCHES/LIBC-0.6/src/gcc/configure 
#  --enable-clh --enable-threads=os2 --enable-shared=libgcc,bfd,opcodes --enable-nls 
#  --without-included-gettext --with-local-prefix=D:/CODING/LIBC/SVN/BRANCHES/LIBC-0.6/TOOLS/x86.os2/gcc/staged 
#  --prefix=/gcc --with-gnu-as --disable-libgcj --enable-languages=c,c++
# Thread model: os2
# gcc version 3.3.5 (Bird Build 2007-06-10 14:30)
# 
#  make -v
# GNU Make version 3.77, by Richard Stallman and Roland McGrath.
# Copyright (C) 1988, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98
# 	Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.
# There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.
# 
# Report bugs to <bug-make@gnu.org>.
# 
#

NAME = flac
LIBS = -L/usr/test/lib -lFLAC++ -lFLAC -logg -lm
INCLUDE  = -I/usr/test/include

CC = g++ -c
LD = g++ -s
RM = rm

CFLAGS   = -Zmts -Wall -O3
TARGET = $(NAME)play

all: $(TARGET).dll

DEF      = $(NAME)play.def
OBJECTS  = $(NAME)play.o id3tag.o decoder.o decoder++.o

$(TARGET).dll : $(OBJECTS) $(DEF)
	$(LD) -Zdll -Zmts $(CFLAGS) -o $(TARGET).dll $(OBJECTS) $(LIBS) $(DEF)

clean:
	-@$(RM) ..\*.dll $(OBJECTS) *.bak ..\*.bak 2> nul

$(NAME)play.o : $(NAME)play.cpp decoder.h
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<

id3tag.o : id3tag.cpp decoder.h
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ id3tag.cpp

decoder.o: decoder.cpp decoder.h Version Build
	$(CC) $(CFLAGS) $(INCLUDE) -DFLACVERSION=\"`cat Version`\" -DFLACBUILD=\"`cat Build`\" -o $@ $<

decoder++.o: decoder++.cpp decoder.h
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<

$(NAME)play.def: $(NAME)play_form.def Version Build Makefile
	VERSION=`cat Version` ; \
	BUILD=`cat Build` ; \
	DATE=`date +"%d %b %Y %X"` ; \
	sed -e 's/^DESCRIPTION .*/DESCRIPTION "@#ntim:'$$VERSION'.'$$BUILD'#@##1## '"$$DATE"'     tim.home::en:ru:'$$BUILD'::@@FLAC player plug-in for PM123 (C)2008 ntim <ntim@softhome.net>"/g' \
	    $< > $@
