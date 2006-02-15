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
NAME   	= ot-db-pgsql
VERSION	= 1.2.0
DEPENDS = db-pgsql
MT-VARIANTS = 1

DLL-DEPENDS = ot-general
EXTLIBS = -lpq

#Only build release versions - debug is not PIC
#Don't override in MinGW
ifndef CROSS-COMPILE
VARIANTS = release single-release
endif

DEBIAN-NAME    = libot-db-pgsql
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 3

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk


####
endif
####
