#==========================================================================
# Makefile for ObTools::Misc
#
# Copyright (c) 2003-2015 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-misc
HEADERS = ot-misc.h
DEPENDS = xml chan mt text
OBJS  	= md5.o crc.o crc32.o proplist.o random.o dumper.o range.o \
          hash-interp.o
TESTS 	= test-prop test-range test-hash-interp
TESTLIB = -lgtest
TESTCMD	= ./test-prop \
       && ./test-range \
       && ./test-hash-interp

SOCKET  = 1  # Because chan needs net

ROOT    = ../..
include $(ROOT)/build/std.mk

