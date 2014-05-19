#==========================================================================
# Makefile for ObTools::Channel
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-chan
HEADERS = ot-chan.h
DEPENDS = net mt
OBJS    = reader.o writer.o error.o tcp-chan.o block-chan.o stream-chan.o \
          string-chan.o dq-chan.o bits.o
TESTLIB = -lgtest
TESTS   = test-block test-stream test-string test-dq test-bits test-limited
TESTCMD = ./test-block && ./test-stream && ./test-string && ./test-dq \
          && ./test-bits && ./test-limited
SOCKET  = 1

# FD-chan is Unix/Mac only
ifndef MINGW
OBJS += fd-chan.o
TESTS += test-fd
TESTCMD += && ./test-fd
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



