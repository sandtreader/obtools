#==========================================================================
# Makefile for ObTools XMLMesh client library
#
# Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-client
HEADERS = ot-xmlmesh-client.h ot-xmlmesh-client-otmp.h
OBJS    = client.o mclient.o transport-otmp.o
DEPENDS = xmlmesh-core xmlmesh-otmp net log xml text soap web misc time \
	  tube mt
SOCKET  = 1

ifdef MINGW
EXTLIBS = -lssl32 -leay32
else
EXTLIBS = -lssl -lcrypto
endif

TESTS 	= test-client test-mclient
TESTCMD = ./test-client localhost

ROOT    = ../..
include $(ROOT)/build/std.mk


