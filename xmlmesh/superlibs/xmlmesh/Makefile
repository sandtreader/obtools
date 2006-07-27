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
NAME      = ot-xmlmesh
VERSION   = 1.5.0
CONTAINS  = xmlmesh-core xmlmesh-otmp xmlmesh-client
DEPENDS   = ot-general
MT-VARIANTS = 1

#Only build release versions - debug is not PIC
#Don't override in MinGW
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-xmlmesh
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT      = ../../..
include $(ROOT)/build/std.mk


