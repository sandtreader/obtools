#==========================================================================
# Makefile for ObTools::Time
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-time
HEADERS = ot-time.h 
OBJS  	= duration.o stamp.o
TESTS 	= test-duration test-stamp
TESTCMD	= ./test-stamp < ../test.stamps.txt \
       && ./test-duration < ../test.durations.txt  
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk


