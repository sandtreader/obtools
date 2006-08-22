#==========================================================================
# Makefile for ObTools Code generation super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME	= ot-codegen
VERSION	= 1.2.0
CONTAINS = cppt regen xmi
DEPENDS = ot-general

#Only build default (MT) version
MULTI	= 1

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-codegen
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 4

ROOT	= ../../..
include $(ROOT)/build/std.mk
