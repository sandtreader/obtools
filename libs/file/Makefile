#==========================================================================
# Makefile for ObTools::File
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-file
HEADERS = ot-file.h 
OBJS  	= path.o directory.o stream.o
DEPENDS = text gen
TESTLIB = -lgtest
TESTS 	= test-path test-dir test-mkdir test-path-gtest test-stream
TESTCMD	= ./test-path ~/.emacs &&                 \
	  ./test-path /foo/bar/wombat/fred.exe && \
          ./test-path /usr/include/linux/ ../../foo && \
	  ./test-dir /usr/bin/ && \
    echo "123456789" > ./test-path-gtest.gpt && sleep 2 && \
   ./test-path-gtest && \
   ./test-stream


ROOT    = ../..
include $(ROOT)/build/std.mk


