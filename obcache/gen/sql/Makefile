#==========================================================================
# Makefile for ObTools ObCache SQL Schema generator
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= ot-generate-schema
OBJS 	= _genschema.o
DEPENDS = xmi regen text
TESTCMD = ./ot-generate-schema < ../../../models/test.xmi

ROOT    = ../../..
include $(ROOT)/build/std.mk

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
_genschema.cc: ../genschema.cfg.xml  \
                $(DIR-xmitoolgen)/build-release/xmitoolgen Makefile
	$(DIR-xmitoolgen)/build-release/xmitoolgen ../genschema.cfg.xml > _genschema.cc


