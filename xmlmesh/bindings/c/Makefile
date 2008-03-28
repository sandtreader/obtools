#==========================================================================
# Makefile for ObTools XMLMesh C binding
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-c
HEADERS = ot-xmlmesh-c.h
OBJS    = xmlmesh-c.o
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp net xml web soap \
          log text misc mt tube
SOCKET = 1

ifdef MINGW
EXTLIBS = -lssl32 -leay32
else
EXTLIBS = -lssl -lcrypto
endif

TESTS	= test-client
TESTCMD = ./test-client localhost

ROOT    = ../../..
include $(ROOT)/build/std.mk

