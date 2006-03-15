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
VERSION   = 1.3.0
CONTAINS  = xmlmesh-core xmlmesh-otmp xmlmesh-client soap
DEPENDS   = ot-general
MT-VARIANTS = 1

#Only build release versions - debug is not PIC
#Don't override in MinGW
ifndef CROSS-COMPILE
VARIANTS = release single-release
endif

DEBIAN-NAME    = libot-xmlmesh
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 2

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT      = ../../..
include $(ROOT)/build/std.mk


