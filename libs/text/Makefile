#==========================================================================
# Makefile for ObTools::Text
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-text
HEADERS = ot-text.h 
OBJS  	= ws.o pattern.o case.o subst.o convert.o base64.o
TESTS 	= test-ws test-pattern test-case test-subst test-convert test-base64
TESTCMD	= ./test-ws && ./test-pattern && ./test-case && ./test-subst \
          && ./test-convert && ./test-base64 hello
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk


