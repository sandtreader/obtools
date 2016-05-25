#==========================================================================
# Makefile for ObTools::Text
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-text
HEADERS = ot-text.h
OBJS    = ws.o pattern.o case.o subst.o convert.o base64.o base36.o \
          base16alpha.o split.o utf8.o

TESTLIB = -lgtest
TESTS   = test-ws-gtest test-utf8 test-convert
TESTCMD = ./test-utf8 && ./test-ws-gtest && ./test-convert

ROOT    = ../..
include $(ROOT)/build/std.mk


