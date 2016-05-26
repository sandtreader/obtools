#==========================================================================
# Makefile for ObTools ObCache in-memory class generator
#
# Copyright (c) 2008 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

# Supressed because warns in clang
#TYPE	= exes
#NAME   	= ot-generate-core-cpp 
#OBJS 	= 
#DEPENDS = xmi regen text misc
#TESTCMD = 

ROOT    = ../../..
include $(ROOT)/build/std.mk

ifdef RELEASE
XMITOOLGEN = $(DIR-xmitoolgen)/build-release$(VARIANT-SUFFIX)/xmitoolgen 
else
XMITOOLGEN = $(DIR-xmitoolgen)/build-debug$(VARIANT-SUFFIX)/xmitoolgen 
endif

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
ot-generate-core-cpp.cc: ../core-cpp.xmitoolgen.xml Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@
