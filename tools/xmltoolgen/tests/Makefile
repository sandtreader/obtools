#==========================================================================
# Makefile for ObTools XML Tool Generator test
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= exe
NAME   	= testtool
OBJS 	= _testtool.o
DEPENDS = xml regen text misc
TESTCMD = ./testtool < ../test.xml
VARIANTS = debug
VARIANT-debug = DEBUG

ROOT    = ../../..
include $(ROOT)/build/std.mk

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
_testtool.cc: ../testtool.cfg.xml ../../build-debug/xmltoolgen Makefile
	../../build-debug/xmltoolgen ../testtool.cfg.xml > _testtool.cc


