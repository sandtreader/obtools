#==========================================================================
# Makefile for ObTools generic database super library
#
# Copyright (c) 2006 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME   	= ot-db2
VERSION	= 2.1.2
CONTAINS = db

DEPENDS = ot-general2

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-db2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 3

ROOT    = ../../..
include $(ROOT)/build/std.mk


