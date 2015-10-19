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
TESTS 	= test-split test-exhaustive-dates test-parse \
          test-duration test-stamp test-date-stamp
TESTLIB = -lgtest
TESTCMD	= ./test-split \
       && ./test-exhaustive-dates \
       && ./test-parse \
       && ./test-stamp < ../test.stamps.txt \
       && ./test-date-stamp < ../test.stamps.txt \
       && ./test-duration < ../test.durations.txt

ROOT    = ../..
include $(ROOT)/build/std.mk


