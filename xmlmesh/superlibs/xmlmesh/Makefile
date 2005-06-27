#==========================================================================
# Makefile for ObTools XMLMesh super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE 	  = superlib
NAME      = ot-xmlmesh
VERSION   = 1.0.0
DEPENDS   = xmlmesh-core xmlmesh-otmp xmlmesh-client soap
MT-VARIANTS = 1

DEBIAN-NAME    = libot-xmlmesh
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 2

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT      = ../../..
include $(ROOT)/build/std.mk


