#==========================================================================
# Makefile for ObTools::JSON
#
# Copyright (c) 2016 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-json
HEADERS = ot-json.h
DEPENDS = text lex
OBJS    = parser.o value.o
TESTS   = test-parser test-value test-real-data
TESTLIB = -lgtest
TESTCMD = ./test-parser && ./test-value && ./test-real-data

ROOT    = ../..
include $(ROOT)/build/std.mk
