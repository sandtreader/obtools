#==========================================================================
# Makefile for ObTools::Time
#
# Copyright (c) 2005 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-time
HEADERS = ot-time.h
OBJS  	= duration.o stamp.o date-stamp.o
DEPENDS = text
TESTS 	= test-split test-exhaustive-dates test-parse
TESTLIB = -lgtest
TESTCMD	= ./test-split \
       && ./test-exhaustive-dates \
       && ./test-parse

ROOT    = ../..
include $(ROOT)/build/std.mk


