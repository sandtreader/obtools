#==========================================================================
# Makefile for ObTools XML Tool Generator test
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= testtool
OBJS 	= _testtool.o
DEPENDS = xml regen
TESTCMD = ./testtool < ../test.xml
VARIANTS = debug
VARIANT-debug = DEBUG

ROOT    = ../../..
include $(ROOT)/build/std.mk

#Template code
_testtool.cc: ../testtool.cfg.xml ../../build-debug/xmltoolgen-cc Makefile
	../../build-debug/xmltoolgen-cc ../testtool.cfg.xml > _testtool.cc


