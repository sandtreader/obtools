#==========================================================================
# Makefile for ObTools MySQL Database super library
#
# Copyright (c) 2006 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
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
HOSTTYPE=$(shell uname -m)
ifdef HOSTTYPE
ifeq ($(HOSTTYPE), x86_64)
LDFLAGS += -L/usr/lib64/mysql
else
LDFLAGS += -L/usr/lib/mysql
endif
endif
LDFLAGS += -L/usr/lib/mysql
endif

EXTLIBS = -lmysqlclient_r -lz

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db-mysql2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 4

ROOT    = ../../..
include $(ROOT)/build/std.mk


