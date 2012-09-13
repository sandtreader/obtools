#==========================================================================
# Makefile for ObTools::Net
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-net
HEADERS = ot-net.h
OBJS  	= socket.o address.o client.o stream.o server.o single-server.o
TESTS 	= test-client test-server test-peer test-single-server
TESTCMD	= ./test-client testhost
DEPENDS = mt
SOCKET  = 1

#In MinGW, add Winsock initialise interface
ifdef MINGW
OBJS    += winsock.o
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



