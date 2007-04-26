#==========================================================================
# Makefile for ObTools XMLMesh command-line message utility
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exes
NAME   	= xmlmesh-send 

ifndef CROSS-COMPILE
NAME += xmlmesh-receive
endif

DEBIAN-NAME    = ot-xmlmesh-cli
DEBIAN-VERSION = 2.0.0
DEBIAN-REVISION = 1

ifdef DEBUG
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
	  net log xml soap web text misc
else
DEPENDS	= ot-xmlmesh2 ot-general2
endif

#Only needs single-threaded
SINGLE = 1
SOCKET = 1

ROOT    = ../../..
include $(ROOT)/build/std.mk

