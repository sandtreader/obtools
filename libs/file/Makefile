#==========================================================================
# Makefile for ObTools::File
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-file
HEADERS = ot-file.h
OBJS    = path.o directory.o stream.o glob.o
DEPENDS = text gen
TESTLIB = -lgtest
TESTS   = test-path test-stream test-glob
TESTCMD = ./test-path && \
          ./test-stream && \
          ./test-glob


ROOT    = ../..
include $(ROOT)/build/std.mk


