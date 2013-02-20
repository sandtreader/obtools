#==========================================================================
# Makefile for ObTools::XML
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-xml
HEADERS = ot-xml.h
OBJS  	= element.o parser.o xpath.o config.o expand.o
DEPENDS = text file
TESTS 	= test-xml test-xpath test-config test-update test-expand dump-xml \
          test-xml-gtest test-xpath-gtest
TESTLIB = -lgtest
TESTCMD	= ./test-xml-gtest && \
          ./test-xpath-gtest && \
	  ./test-xml < ../tests/simple.xml && \
          ./test-xpath < ../tests/simple.xml && \
	  ./test-config && \
	  ./test-update && \
	  ./test-expand < ../tests/expand.xml

ROOT    = ../..
include $(ROOT)/build/std.mk


