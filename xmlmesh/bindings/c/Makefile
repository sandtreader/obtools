#==========================================================================
# Makefile for ObTools XMLMesh C binding
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-c
HEADERS = ot-xmlmesh-c.h
OBJS    = xmlmesh-c.o
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp net xml web soap \
          log text misc mt tube
SOCKET = 1

TESTS	= test-client
TESTCMD = ./test-client testhost

ROOT    = ../../..
include $(ROOT)/build/std.mk

