#==========================================================================
# Makefile for ObTools::Misc
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-misc
HEADERS = ot-misc.h
OBJS  	= md5.o proplist.o random.o dumper.o pkcs5.o
TESTS 	= test-md5 test-prop test-random test-dump test-pkcs5
TESTCMD	= ./test-md5 && ./test-prop && ./test-random && ./test-dump \
	  && ./test-pkcs5
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

