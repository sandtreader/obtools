#==========================================================================
# Makefile for ObTools XML Tool Generator test
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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


