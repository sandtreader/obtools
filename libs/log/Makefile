#==========================================================================
# Makefile for ObTools::Log
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-log
HEADERS = ot-log.h
OBJS  	= distributor.o filter.o logstream.o channel.o logger.o
DEPENDS = text
TESTS 	= test-log
TESTCMD	= ./test-log
MT-VARIANTS = 1

#Suppress MT in SINGLE build
ifndef SINGLE
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk




