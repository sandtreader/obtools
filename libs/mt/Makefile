#==========================================================================
# Makefile for ObTools::MT
#
# Copyright (c) 2003-2016 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-mt
HEADERS = ot-mt.h
OBJS    = thread.o pool.o dqueue.o
DEPENDS = gen
TESTLIB = -lgtest
TESTS   = test-thread test-rwmutex test-dqueue test-pool
TESTCMD = ./test-thread && ./test-rwmutex && ./test-dqueue && ./test-pool

ROOT    = ../..
include $(ROOT)/build/std.mk
