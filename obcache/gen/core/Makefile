#==========================================================================
# Makefile for ObTools ObCache in-memory class generator
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exes
NAME   	= ot-generate-core-cpp 
OBJS 	= 
DEPENDS = xmi regen text misc
TESTCMD = 

ROOT    = ../../..
include $(ROOT)/build/std.mk

XMITOOLGEN = $(DIR-xmitoolgen)/build-release/xmitoolgen 

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
ot-generate-core-cpp.cc: ../core-cpp.xmitoolgen.xml Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@
