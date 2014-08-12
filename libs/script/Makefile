#==========================================================================
# Makefile for XML scripting library
#
# Copyright (c) 2006 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-script
HEADERS = ot-script.h
OBJS  	= language.o baselang.o script.o \
          sequence.o repeat.o parallel.o replicate.o \
          log.o delay.o scope.o set.o thread.o random.o
DEPENDS = xml log text misc init time mt
TESTS 	= test-script
TESTCMD	= ./test-script ../tests/test.xml

ROOT    = ../..
include $(ROOT)/build/std.mk



