#==========================================================================
# Makefile for CPPT
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
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



