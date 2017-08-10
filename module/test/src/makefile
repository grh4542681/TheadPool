# zlog makefile
# Copyright (C) 2010-2012 Hardy Simpson <HardySimpson1984@gmail.com>
# This file is released under the LGPL 2.1 license, see the COPYING file

PLATHOME=/home/helsinki/fram
COMPONENT=heartbeat
BINSPATH=$(PLATHOME)/bin
LIBPATH=$(PLATHOME)/lib
OBJPATH=$(PLATHOME)/module/$(COMPONENT)/obj
INCPATH=$(PLATHOME)/module/$(COMPONENT)/inc
PUBINCPATH=$(PLATHOME)/module/public/inc

OBJ=    \
	Tpooltest.o	\
	initest.o	\
	ipctest.o

BINS=Tpooltest initest ipctest

MV=mv
CP=cp

# Fallback to gcc when $CC is not in $PATH.
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
OPTIMIZATION=-O2
WARNINGS=-Wall -Wstrict-prototypes -fwrapv
DEBUG= -g -ggdb
REAL_CFLAGS= $(OPTIMIZATION) -fPIC -pthread $(CFLAGS) $(WARNINGS) $(DEBUG) -D_GNU_SOURCE
REAL_LDFLAGS=$(LDFLAGS) -L$(LIBPATH) -lrt -lm -pthread -llog -lpub -lJSON -lTpool -lpINI -lipc
REAL_INCLUDE=-I$(INCPATH) -I$(PLATHOME)/module/log/inc -I$(PUBINCPATH) -I$(PLATHOME)/module/json/inc -I$(PLATHOME)/module/threadpool/inc -I$(PLATHOME)/module/ini/inc -I$(PLATHOME)/module/ipc/inc

all: $(OBJ) $(BINS)
	$(MV) *.o $(OBJPATH)

# Binaries:
Tpooltest: Tpooltest.o 
	$(CC) -o $@ $(REAL_INCLUDE) Tpooltest.o  $(REAL_LDFLAGS)


.c.o:
	$(CC) -std=c99 -pedantic -c $(REAL_INCLUDE) $(REAL_CFLAGS) $<

clean:
	rm -rf $(DYLIBNAME) $(STLIBNAME) $(BINS) *.o 

dep:
	$(CC) -MM *.c

32bit:
	@echo ""
	@echo "WARNING: if this fails under Linux you probably need to install libc6-dev-i386"
	@echo ""
	$(MAKE) CFLAGS="-m32" LDFLAGS="-m32"

gprof:
	$(MAKE) CFLAGS="-pg" LDFLAGS="-pg"

gcov:
	$(MAKE) CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs"

coverage: gcov
	make check
	mkdir -p tmp/lcov
	lcov -d . -c -o tmp/lcov/hiredis.info
	genhtml --legend -o tmp/lcov/report tmp/lcov/hiredis.info

noopt:
	$(MAKE) OPTIMIZATION=""


.PHONY: all clean dep 32bit gprof gcov noopt
