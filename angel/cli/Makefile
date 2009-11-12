#==========================================================================
# Makefile for angel command line utility
#
# Copyright (c) 2005 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= exe
NAME    = angel
OBJS 	= angel.o
RESOLVER = 1

DEBIAN-NAME    = angel
DEBIAN-VERSION = 0.1.0
DEBIAN-REVISION = 1

DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
          net log mt xml text init misc

ifdef RELEASE
CPPFLAGS += -DDAEMON
endif

CONFIGS = angel.cfg.xml
TESTCMD = ./angel ../angel.cfg.xml

ROOT    = ../..
include $(ROOT)/build/std.mk
