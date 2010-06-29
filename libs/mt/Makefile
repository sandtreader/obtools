#==========================================================================
# Makefile for ObTools::MT
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-mt
HEADERS = ot-mt.h
OBJS  	= thread.o pool.o dqueue.o
TESTS 	= test-thread test-pool test-queue test-mutex test-rmutex \
	  test-rwmutex test-mqueue test-dqueue
TESTCMD	= ./test-thread 10 && ./test-queue && ./test-mqueue && ./test-dqueue

ROOT    = ../..
include $(ROOT)/build/std.mk




