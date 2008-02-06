#==========================================================================
# Makefile for ObTools XMI Tool Generator test
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= testtool
OBJS 	= _testtool.o
DEPENDS = xmi xml regen text misc
TESTCMD = ./testtool ../xmi2cc.cfg.xml < ../test.xmi
VARIANTS = debug
VARIANT-debug = DEBUG

ROOT    = ../../..
include $(ROOT)/build/std.mk

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
_testtool.cc: ../testtool.cfg.xml ../../build-debug/xmitoolgen Makefile
	../../build-debug/xmitoolgen ../testtool.cfg.xml > _testtool.cc


