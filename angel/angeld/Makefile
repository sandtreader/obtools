#==========================================================================
# Makefile for angeld process manager
#
# Copyright (c) 2005 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= exe
NAME    = angeld
HEADERS = angeld.h
OBJS 	= angeld.o server.o process.o
CRYPTO  = 1
RESOLVER = 1

DEBIAN-NAME    = angeld
DEBIAN-VERSION = 0.2.0
DEBIAN-REVISION = 1

DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
          net log mt xml soap web text cache init misc file

ifdef RELEASE
CPPFLAGS += -DDAEMON
endif

CONFIGS = angeld.cfg.xml

ROOT    = ../..
include $(ROOT)/build/std.mk
