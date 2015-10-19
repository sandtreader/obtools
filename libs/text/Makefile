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
TESTS   = test-ws-gtest test-ws test-pattern test-case test-subst test-convert\
          test-base64 test-base36 test-base16alpha test-split
TESTCMD = ./test-ws-gtest && ./test-ws && ./test-pattern && ./test-case\
          && ./test-subst \
          && ./test-convert && ./test-base64 hello \
          && ./test-base36 0 1 1000 1000000 1000000000 \
          && ./test-base16alpha 0 1 1000 1000000 1000000000 \
          && ./test-split

ROOT    = ../..
include $(ROOT)/build/std.mk


