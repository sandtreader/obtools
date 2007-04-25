#==========================================================================
# Makefile for ObTools generic database super library
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME   	= ot-db
VERSION	= 2.1.0
CONTAINS = db
MT-VARIANTS = 1

DEPENDS = ot-general

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk


