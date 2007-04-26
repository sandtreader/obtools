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
MT-VARIANTS = 1

DEPENDS = ot-general2 ot-db2

ifdef SINGLE
EXTLIBS = -lmysqlclient -lz
else
EXTLIBS = -lmysqlclient_r -lz
endif

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db-mysql2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 2

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk


