#==========================================================================
# Makefile for ObTools XMLMesh command-line message utility
#
# Copyright (c) 2005 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= exes
NAME   	= xmlmesh-receive

DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
	  net log xml soap web text misc mt tube

SOCKET = 1

ifdef MINGW
EXTLIBS = -lssl32 -leay32
else
EXTLIBS = -lssl -lcrypto
endif

ROOT    = ../../../..
include $(ROOT)/build/std.mk

