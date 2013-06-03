#==========================================================================
# Makefile for ObTools::Expression
#
# Copyright (c) 2013 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-expr
HEADERS = ot-expr.h
DEPENDS = text misc
OBJS    = token.o eval.o
TESTS   = test-token test-eval test-calc
TESTLIB = -lgtest
TESTCMD = ./test-token && ./test-eval

ROOT    = ../..
include $(ROOT)/build/std.mk
