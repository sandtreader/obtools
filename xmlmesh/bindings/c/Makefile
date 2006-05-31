#==========================================================================
# Makefile for ObTools XMLMesh C binding
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-c
HEADERS = ot-xmlmesh-c.h
OBJS    = xmlmesh-c.o
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp net xml web soap \
          log text misc
MT-VARIANTS = 1
SOCKET = 1

# Suppress MT library if single 
ifndef SINGLE
DEPENDS += mt
endif

TESTS	= test-client
TESTCMD = ./test-client localhost

ROOT    = ../../..
include $(ROOT)/build/std.mk

