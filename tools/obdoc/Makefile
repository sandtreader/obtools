#==========================================================================
# Makefile for ObTools ObDoc Documentor
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= obdoc
OBJS 	= _obdoc.o
DEPENDS = xml regen text
TESTCMD = ./obdoc < ../test.obdoc.xml
VARIANTS = debug
VARIANT-debug = DEBUG

ROOT    = ../..
include $(ROOT)/build/std.mk

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
_obdoc.cc: ../obdoc.cfg.xml $(DIR-xmltoolgen)/build-debug/xmltoolgen Makefile
	$(DIR-xmltoolgen)/build-debug/xmltoolgen ../obdoc.cfg.xml > _obdoc.cc


