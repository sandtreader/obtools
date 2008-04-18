#==========================================================================
# Makefile for ObTools::SSL
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-ssl
HEADERS = ot-ssl.h
OBJS  	= context.o socket.o client.o server.o
TESTS 	= test-client test-server
DEPENDS = mt net log crypto
TESTCMD	= 
SOCKET = 1

ifdef MINGW
EXTLIBS = -lssl32 -leay32
else
EXTLIBS = -lssl -lcrypto
endif

ROOT    = ../..
include $(ROOT)/build/std.mk

