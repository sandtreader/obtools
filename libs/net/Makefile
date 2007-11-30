#==========================================================================
# Makefile for ObTools::Net
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-net
HEADERS = ot-net.h
OBJS  	= socket.o address.o client.o stream.o server.o single-server.o
TESTS 	= test-client test-server test-peer test-single-server
TESTCMD	= ./test-client localhost
DEPENDS = mt
SOCKET  = 1

#In MinGW, add Winsock initialise interface
ifdef MINGW
OBJS    += winsock.o
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



