#==========================================================================
# Makefile for angel command line utility
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME    = angel
OBJS 	= angel.o

DEBIAN-NAME    = angel
DEBIAN-VERSION = 0.1.0
DEBIAN-REVISION = 1

#Build from static local libraries in DEBUG, installed .so's in release
ifdef DEBUG
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
          net log mt xml text init misc
else
DEPENDS = ot-general ot-xmlmesh
endif

ifdef RELEASE
CPPFLAGS += -DDAEMON
endif

CONFIGS = angel.cfg.xml
TESTCMD = ./angel ../angel.cfg.xml
MULTI   = 1

ROOT    = ../..
include $(ROOT)/build/std.mk
