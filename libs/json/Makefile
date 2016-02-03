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
OBJS    = parser.o
TESTS   = test-parser
TESTLIB = -lgtest
TESTCMD = ./test-parser

ROOT    = ../..
include $(ROOT)/build/std.mk
