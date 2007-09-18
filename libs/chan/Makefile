#==========================================================================
# Makefile for ObTools::Channel
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-chan
HEADERS = ot-chan.h
DEPENDS = net mt
OBJS  	= reader.o writer.o error.o tcp-chan.o block-chan.o stream-chan.o \
	  string-chan.o
TESTS 	= test-block test-stream test-string
TESTCMD = ./test-block && ./test-stream && ./test-string
SOCKET  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



