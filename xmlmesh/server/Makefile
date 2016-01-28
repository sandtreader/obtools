#==========================================================================
# Makefile for ObTools XMLMesh server
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= exe
NAME    = xmlmesh-server
HEADERS = server.h
OBJS 	= main.o server.o service.o mpath.o otmp-server.o otmp-client.o \
	  correlator.o publisher.o http-server.o

DEBIAN-NAME    = ot-xmlmesh
DEBIAN-VERSION = 3.2.0
DEBIAN-REVISION = 1~dev1

DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
          net log mt xml soap web text cache init misc tube file

ifdef RELEASE
ifndef MINGW
CPPFLAGS += -DDAEMON
endif
endif

CPPFLAGS += -DOT_XMLMESH_VERSION=$(DEBIAN-VERSION)

CONFIGS = xmlmesh.cfg.xml
SOCKET  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk
