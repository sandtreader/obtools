#==========================================================================
# Makefile for ObTools::MT
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-mt
HEADERS = ot-mt.h
OBJS  	= thread.o pool.o
TESTS 	= test-thread test-pool test-queue test-mutex test-rmutex \
	  test-rwmutex test-mqueue
TESTCMD	= ./test-thread 10 && ./test-queue && ./test-mqueue

ROOT    = ../..
include $(ROOT)/build/std.mk




