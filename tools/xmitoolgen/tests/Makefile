#==========================================================================
# Makefile for ObTools XMI Tool Generator test
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= testtool
OBJS 	= _testtool.o
DEPENDS = xmi xml regen
TESTCMD = ./testtool < ../test.xmi
VARIANTS = debug
VARIANT-debug = DEBUG

ROOT    = ../../..
include $(ROOT)/build/std.mk

#Template code
_testtool.cc: ../testtool.cfg.xml ../../build-debug/xmitoolgen-cc Makefile
	../../build-debug/xmitoolgen-cc ../testtool.cfg.xml > _testtool.cc


