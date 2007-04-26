#==========================================================================
# Makefile for ObTools Postgres Database super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
MT-VARIANTS = 1

DEPENDS = ot-general2 ot-db2
EXTLIBS = -lpq

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db-pgsql2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk


####
endif
####
