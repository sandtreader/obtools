#==========================================================================
# Makefile for ObTools::File
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-file
HEADERS = ot-file.h 
OBJS  	= path.o directory.o
DEPENDS = text
TESTS 	= test-path test-dir
TESTCMD	= ./test-path /foo/bar/wombat/fred.exe && \
          ./test-path /usr/include/linux/ ../../foo && \
	  ./test-dir /usr/bin/
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk


