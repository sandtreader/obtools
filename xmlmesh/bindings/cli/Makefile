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
DEBIAN-VERSION = 2.1.0
DEBIAN-REVISION = 1

DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp \
	  net log xml soap web text misc mt tube

SOCKET = 1

ROOT    = ../../..
include $(ROOT)/build/std.mk

