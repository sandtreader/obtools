#==========================================================================
# Makefile for angeld process manager
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME    = angeld
HEADERS = angeld.h
OBJS 	= angeld.o server.o process.o

DEBIAN-NAME    = angeld
DEBIAN-VERSION = 0.1.0
DEBIAN-REVISION = 1

#Build from static local libraries in DEBUG, installed .so's in release
ifdef DEBUG
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
          net log mt xml soap web text cache init misc file
else
DEPENDS = ot-general ot-xmlmesh
endif

ifdef RELEASE
CPPFLAGS += -DDAEMON
endif

CONFIGS = angeld.cfg.xml
MULTI   = 1

ROOT    = ../..
include $(ROOT)/build/std.mk
