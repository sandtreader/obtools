#==========================================================================
# Makefile for ObTools::Net
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-net
HEADERS = ot-net.h
OBJS  	= socket.o address.o client.o stream.o
TESTS 	= test-client 
TESTCMD	= ./test-client localhost
MT-VARIANTS = 1

#Catch ourselves recursing in SINGLE mode and suppress server and use of MT
ifndef SINGLE
OBJS 	+= server.o
TESTS  	+= test-server
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



