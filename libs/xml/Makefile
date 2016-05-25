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
TESTS 	= test-xml-gtest test-xpath-gtest
TESTLIB = -lgtest
TESTCMD	= ./test-xml-gtest && ./test-xpath-gtest

ROOT    = ../..
include $(ROOT)/build/std.mk


