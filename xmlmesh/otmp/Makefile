#==========================================================================
# Makefile for ObTools XMLMesh OTMP protocol library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

NAME   	= ot-xmlmesh-otmp
HEADERS = ot-xmlmesh-otmp.h
OBJS    = client.o
DEPENDS = net log text
MT-VARIANTS = 1

TESTS 	= test-client
TESTCMD = ./test-client localhost

# Suppress MT library and server if single 
ifndef SINGLE
DEPENDS += mt
OBJS 	+= server.o
TESTS 	+= test-server
endif

ROOT    = ../..
include $(ROOT)/build/lib.mk



