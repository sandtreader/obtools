#==========================================================================
# Makefile for ObTools::Channel
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-chan
HEADERS = ot-chan.h
DEPENDS = net
OBJS  	= reader.o writer.o error.o tcp-chan.o block-chan.o stream-chan.o
TESTS 	= test-block test-stream
TESTCMD = ./test-block && ./test-stream
MT-VARIANTS = 1
SOCKET  = 1

#Catch ourselves recursing in SINGLE mode and suppress use of MT
ifndef SINGLE
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



