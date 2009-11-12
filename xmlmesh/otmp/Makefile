#==========================================================================
# Makefile for ObTools XMLMesh OTMP protocol library
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-otmp
OBJS 	= server.o
HEADERS = ot-xmlmesh-otmp.h
DEPENDS = net log text mt tube
SOCKET  = 1

TESTS 	= test-client test-server
TESTCMD = ./test-client localhost

ROOT    = ../..
include $(ROOT)/build/std.mk



