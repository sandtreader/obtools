#==========================================================================
# Makefile for ObTools Code generation super library
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME	= ot-codegen2
VERSION	= 1.2.0
CONTAINS = cppt regen xmi
DEPENDS = ot-general2

#Only build default (MT) version
MULTI	= 1

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-codegen2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 4

ROOT	= ../../..
include $(ROOT)/build/std.mk
