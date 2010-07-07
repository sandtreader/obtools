#==========================================================================
# Makefile for ObTools::Tube
#
# Copyright (c) 2007 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-tube
HEADERS = ot-tube.h
OBJS  	= server.o sync-server.o client.o sync-request.o \
	  sync-client.o auto-sync-client.o bi-sync-server.o message.o
TESTS 	= test-server test-sync-server test-client test-bi-sync-server
TESTCMD	= 
DEPENDS = misc net log mt chan text ssl ssl-openssl file cache
SOCKET  = 1
CRYPTO  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



