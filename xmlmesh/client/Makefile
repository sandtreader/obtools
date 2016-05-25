#==========================================================================
# Makefile for ObTools XMLMesh client library
#
# Copyright (c) 2007 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-client
HEADERS = ot-xmlmesh-client.h ot-xmlmesh-client-otmp.h
OBJS    = client.o mclient.o transport-otmp.o
DEPENDS = xmlmesh-core xmlmesh-otmp net log xml text soap web misc time \
	  tube mt msg
SOCKET  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk


