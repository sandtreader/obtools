#==========================================================================
# Makefile for ObTools XMLMesh client library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

NAME   	= ot-xmlmesh-client
HEADERS = ot-xmlmesh-client.h ot-xmlmesh-client-otmp.h
OBJS    = client.o transport-otmp.o
DEPENDS = xmlmesh-core xmlmesh-otmp net log xml text 
MULTI   = 1

#Can't build single yet - need MT::Queue to be available even without threading
#VARIANT-SINGLE = 1

TESTS 	= test-client
TESTCMD = ./test-client localhost

# Suppress MT library and MultiClient if single 
ifndef SINGLE
DEPENDS += mt
OBJS 	+= mclient.o
TESTS 	+= test-mclient
endif

ROOT    = ../..
include $(ROOT)/build/lib.mk


