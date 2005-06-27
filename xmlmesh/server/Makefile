#==========================================================================
# Makefile for ObTools XMLMesh server
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME    = xmlmesh-server
HEADERS = server.h
OBJS 	= main.o server.o service.o mpath.o otmp-server.o otmp-client.o \
	  correlator.o publisher.o

DEBIAN-NAME    = ot-xmlmesh
DEBIAN-VERSION = 1.0.0
DEBIAN-REVISION = 2

#Build from static local libraries in DEBUG, installed .so's in release
ifdef DEBUG
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
          net log mt xml soap text cache init misc
else
DEPENDS = ot-general ot-xmlmesh
endif

ifdef RELEASE
CPPFLAGS += -DDAEMON
endif

CONFIGS = xmlmesh.cfg.xml
MULTI   = 1

ROOT    = ../..
include $(ROOT)/build/std.mk
