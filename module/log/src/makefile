# zlog makefile
# Copyright (C) 2010-2012 Hardy Simpson <HardySimpson1984@gmail.com>
# This file is released under the LGPL 2.1 license, see the COPYING file

PLATHOME=/home/helsinki/fram
COMPONENT=log
BINSPATH=$(PLATHOME)/bin
LIBPATH=$(PLATHOME)/lib
OBJPATH=$(PLATHOME)/module/$(COMPONENT)/obj
INCPATH=$(PLATHOME)/module/$(COMPONENT)/inc
PUBINCPATH=$(PLATHOME)/module/public/inc

OBJ=    \
  buf.o    \
  category.o    \
  category_table.o    \
  conf.o    \
  event.o    \
  format.o    \
  level.o    \
  level_list.o    \
  mdc.o    \
  record.o    \
  record_table.o    \
  rotater.o    \
  rule.o    \
  spec.o    \
  thread.o    \
  zc_arraylist.o    \
  zc_hashtable.o    \
  zc_profile.o    \
  zc_util.o    \
  zlog.o	\
  log.o
BINS=zlog-chk-conf
LIBNAME=liblog

MV=mv
CP=cp

# Fallback to gcc when $CC is not in $PATH.
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
OPTIMIZATION=-O2
WARNINGS=-Wall -Werror -Wstrict-prototypes -fwrapv
DEBUG= -g -ggdb
REAL_CFLAGS= -I$(INCPATH) $(OPTIMIZATION) -fPIC -pthread $(CFLAGS) $(WARNINGS) $(DEBUG)
REAL_LDFLAGS=$(LDFLAGS) -pthread 

DYLIBSUFFIX=so
STLIBSUFFIX=a
DYLIBNAME=$(LIBNAME).$(DYLIBSUFFIX)
DYLIB_MAKE_CMD=$(CC) -shared -o $(DYLIBNAME) $(LDFLAGS)
STLIBNAME=$(LIBNAME).$(STLIBSUFFIX)
STLIB_MAKE_CMD=ar rcs $(STLIBNAME)

# Platform-specific overrides
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifeq ($(uname_S),SunOS)
#  REAL_LDFLAGS+= -ldl -lnsl -lsocket
  DYLIB_MAKE_CMD=$(CC) -G -o $(DYLIBNAME) -h $(DYLIB_MINOR_NAME) $(LDFLAGS)
  INSTALL= cp -r
endif
ifeq ($(uname_S),Darwin)
  DYLIBSUFFIX=dylib
  DYLIB_MINOR_NAME=$(LIBNAME).$(ZLOG_MAJOR).$(ZLOG_MINOR).$(DYLIBSUFFIX)
  DYLIB_MAJOR_NAME=$(LIBNAME).$(ZLOG_MAJOR).$(DYLIBSUFFIX)
  DYLIB_MAKE_CMD=$(CC) -dynamiclib -install_name $(INSTALL_LIBRARY_PATH)/$(DYLIB_MINOR_NAME) -o $(DYLIBNAME) $(LDFLAGS)
endif

ifeq ($(uname_S),AIX)
  # this logic of minor major is not relevant on AIX or at least not widely used
  # not to mention dynamic linker .a preference...
  DYLIB_MAKE_CMD=$(CC) -shared -Wl,-G,-b64 -maix64 -pthread -o $(DYLIBNAME) $(LDFLAGS)
  REAL_CFLAGS+= -maix64
  STLIB_MAKE_CMD=OBJECT_MODE=64 ar rcs $(STLIBNAME) $(DYLIB_MAJOR_NAME)
endif

all: $(STLIBNAME) $(DYLIBNAME) $(BINS)
	$(MV) *.o $(OBJPATH)
	$(MV) *.a $(LIBPATH)
	$(MV) *.so $(LIBPATH)

$(DYLIBNAME): $(OBJ)
	$(DYLIB_MAKE_CMD) $(OBJ) $(REAL_LDFLAGS)

$(STLIBNAME): $(OBJ)
	$(STLIB_MAKE_CMD) $(OBJ)

dynamic: $(DYLIBNAME)
static: $(STLIBNAME)

# Binaries:
zlog-chk-conf: zlog-chk-conf.o $(STLIBNAME) $(DYLIBNAME)
	$(CC) -o $@ zlog-chk-conf.o -L. -llog $(REAL_LDFLAGS)
	$(MV) zlog-chk-conf $(BINSPATH)

.c.o:
	$(CC) -std=c99 -pedantic -c $(REAL_CFLAGS) $<

clean:
	rm -rf $(DYLIBNAME) $(STLIBNAME) $(BINS) *.o *.gcda *.gcno *.gcov $(DYLIB_MINOR_NAME)

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
