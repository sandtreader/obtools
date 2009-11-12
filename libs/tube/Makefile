#==========================================================================
# Makefile for ObTools::Tube
#
# Copyright (c) 2007 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-tube
HEADERS = ot-tube.h
OBJS  	= server.o sync-server.o client.o sync-client.o auto-sync-client.o \
	  message.o
TESTS 	= test-server test-sync-server test-client test-sync-client
TESTCMD	= 
DEPENDS = misc net log mt chan text ssl
SOCKET  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



