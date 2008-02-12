#==========================================================================
# Makefile for ObTools ObCache in-memory class generator for C++
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exes
NAME   	= ot-generate-memory-cpp
OBJS 	= 
DEPENDS = xmi regen text misc
TESTCMD = ./ot-generate-memory-cpp ../ot-generate-memory-cpp.cfg.xml < ../../../models/test.xmi 

ROOT    = ../../..
include $(ROOT)/build/std.mk

XMITOOLGEN = $(DIR-xmitoolgen)/build-release/xmitoolgen 

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
ot-generate-memory-cpp.cc: ../memory.xmitoolgen.xml  \
                Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@

