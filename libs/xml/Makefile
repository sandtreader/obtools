#==========================================================================
# Makefile for ObTools::XML
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xml
HEADERS = ot-xml.h
OBJS  	= element.o parser.o xpath.o config.o
TESTS 	= test-xml test-xpath test-config test-update
TESTCMD	= ./test-xml < ../tests/simple.xml && \
          ./test-xpath < ../tests/simple.xml && \
	  ./test-config && \
	  ./test-update
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk


