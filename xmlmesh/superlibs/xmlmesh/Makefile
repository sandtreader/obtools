#==========================================================================
# Makefile for ObTools XMLMesh super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME      = ot-xmlmesh2
VERSION   = 2.3.0
CONTAINS  = xmlmesh-core xmlmesh-otmp xmlmesh-client
DEPENDS   = ot-general2

#Only build release versions - debug is not PIC
#Don't override in MinGW
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-xmlmesh2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ROOT      = ../../..
include $(ROOT)/build/std.mk


