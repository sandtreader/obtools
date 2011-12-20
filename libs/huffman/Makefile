#==========================================================================
# Makefile for ObTools::Huffman
#
# Copyright (c) 2011 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-huffman
HEADERS = ot-huffman.h
DEPENDS = chan text
OBJS    = node.o reader.o tree.o value.o
TESTS   = test-reader test-tree test-value
TESTLIB = -lgtest
TESTCMD = ./test-reader && ./test-tree && ./test-value

ROOT    = ../..
include $(ROOT)/build/std.mk
