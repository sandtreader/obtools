#==========================================================================
# Makefile for ObTools XMLMesh OTMP protocol library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-otmp
HEADERS = ot-xmlmesh-otmp.h
DEPENDS = net log text
MT-VARIANTS = 1

TESTS 	= test-client
TESTCMD = ./test-client localhost


# Suppress MT library and server if single, build single-threaded client
ifdef SINGLE
OBJS   	= client-single.o
else
OBJS 	= client.o server.o
DEPENDS += mt
TESTS 	+= test-server
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



