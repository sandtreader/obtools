#==========================================================================
# Makefile for ObTools Postgres Database super library
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

###Doesn't currently build in Windows - linkage problems with libpq
ifdef CROSS-COMPILE
targets:
else
###

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME   	= ot-db-pgsql2
VERSION	= 2.0.0
CONTAINS = db-pgsql

DEPENDS = ot-general2 ot-db2

ifdef OSX
EXTINCS = /usr/local/pgsql/include/
LDFLAGS += -L/usr/local/pgsql/lib
endif

EXTLIBS = -lpq

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db-pgsql2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 2

ROOT    = ../../..
include $(ROOT)/build/std.mk


####
endif
####
