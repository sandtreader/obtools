#==========================================================================
# Makefile for ObTools XMLMesh command-line message utility
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exes
NAME   	= xmlmesh-send xmlmesh-receive

DEBIAN-NAME    = ot-xmlmesh-cli
DEBIAN-VERSION = 1.2.0
DEBIAN-REVISION = 1

ifdef DEBUG
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
	  net log xml soap text cache misc
else
DEPENDS	= ot-xmlmesh ot-general
endif

#Only needs single-threaded
SINGLE = 1

ROOT    = ../../..
include $(ROOT)/build/std.mk

