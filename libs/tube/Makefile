#==========================================================================
# Makefile for ObTools::Tube
#
# Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

EXTLIBS = -lssl

ROOT    = ../..
include $(ROOT)/build/std.mk



