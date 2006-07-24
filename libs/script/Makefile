#==========================================================================
# Makefile for XML scripting library
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-script
HEADERS = ot-script.h
OBJS  	= language.o baselang.o script.o \
          sequence.o repeat.o parallel.o replicate.o \
          log.o delay.o
DEPENDS = xml log text misc init time
TESTS 	= test-script
TESTCMD	= ./test-script ../tests/test.xml
MT-VARIANTS = 1

ifndef SINGLE
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



