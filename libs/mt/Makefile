#==========================================================================
# Makefile for ObTools::MT
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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




