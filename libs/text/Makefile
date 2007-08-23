#==========================================================================
# Makefile for ObTools::Text
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-text
HEADERS = ot-text.h 
OBJS  	= ws.o pattern.o case.o subst.o convert.o base64.o base36.o \
	  base16alpha.o split.o
TESTS 	= test-ws test-pattern test-case test-subst test-convert     \
          test-base64 test-base36 test-base16alpha test-split
TESTCMD	= ./test-ws && ./test-pattern && ./test-case && ./test-subst \
          && ./test-convert && ./test-base64 hello \
          && ./test-base36 0 1 1000 1000000 1000000000 \
          && ./test-base16alpha 0 1 1000 1000000 1000000000 \
          && ./test-split

ROOT    = ../..
include $(ROOT)/build/std.mk


