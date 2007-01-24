#==========================================================================
# Makefile for ObTools::Misc
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-misc
HEADERS = ot-misc.h
DEPENDS = xml
OBJS  	= md5.o crc.o crc32.o proplist.o random.o dumper.o range.o
TESTS 	= test-md5 test-crc test-crc32 test-prop test-random \
          test-dump test-range 
TESTCMD	= ./test-md5 && ./test-crc hello && ./test-crc32 hello \
       && ./test-prop && ./test-random \
       && ./test-dump && ./test-range < ../test-range.dat
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

