#==========================================================================
# Makefile for ObTools XMLMesh client library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-client
HEADERS = ot-xmlmesh-client.h ot-xmlmesh-client-otmp.h
OBJS    = client.o transport-otmp.o
DEPENDS = xmlmesh-core xmlmesh-otmp net log xml text soap misc
MT-VARIANTS = 1

TESTS 	= test-client
TESTCMD = ./test-client localhost

# Suppress MT library and MultiClient if single 
ifndef SINGLE
DEPENDS += mt
OBJS 	+= mclient.o
TESTS 	+= test-mclient
endif

ROOT    = ../..
include $(ROOT)/build/std.mk


