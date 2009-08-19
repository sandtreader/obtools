#==========================================================================
# Makefile for ObTools MySQL Database super library
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME   	= ot-db-mysql2
VERSION	= 2.0.0
CONTAINS = db-mysql

DEPENDS = ot-general2 ot-db2

ifdef OSX
EXTINCS = /usr/local/mysql/include/
LDFLAGS += -L/usr/local/mysql/lib
endif

ifdef CENTOS
EXTINCS = /usr/include/mysql
LDFLAGS += -L/usr/lib/mysql
endif

EXTLIBS = -lmysqlclient_r -lz

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db-mysql2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 3

ROOT    = ../../..
include $(ROOT)/build/std.mk


