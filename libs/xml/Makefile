#==========================================================================
# Makefile for ObTools::XML
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xml
HEADERS = ot-xml.h
OBJS  	= element.o parser.o xpath.o config.o expand.o
TESTS 	= test-xml test-xpath test-config test-update test-expand
TESTCMD	= ./test-xml < ../tests/simple.xml && \
          ./test-xpath < ../tests/simple.xml && \
	  ./test-config && \
	  ./test-update && \
	  ./test-expand < ../tests/expand.xml

ROOT    = ../..
include $(ROOT)/build/std.mk


