#==========================================================================
# Makefile for ObTools XMLMesh super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE 	  = superlib
NAME      = ot-xmlmesh
VERSION   = 1.3.0
DEPENDS   = xmlmesh-core xmlmesh-otmp xmlmesh-client soap
MT-VARIANTS = 1

#Only build release versions - debug is not PIC
VARIANTS = release single-release

DEBIAN-NAME    = libot-xmlmesh
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT      = ../../..
include $(ROOT)/build/std.mk


