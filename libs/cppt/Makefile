#==========================================================================
# Makefile for CPPT
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-cppt
HEADERS = ot-cppt.h
OBJS  	= processor.o tokenr.o
TESTS 	= test-cppt
TESTCMD	= ./test-cppt < ../tests/simple.cppt > _temp.cc && \
	  cat _temp.cc                                  && \
	  gcc _temp.cc -o _temp -lstdc++                && \
	  ./_temp

ROOT    = ../..
include $(ROOT)/build/std.mk



