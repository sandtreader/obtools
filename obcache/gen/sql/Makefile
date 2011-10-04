#==========================================================================
# Makefile for ObTools ObCache storage generator for SQL
#
# Copyright (c) 2008 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= exes
NAME   	= ot-generate-sql-schema \
          ot-generate-sql-cpp
OBJS 	= 
DEPENDS = xmi regen text misc
TESTCMD = ./ot-generate-sql-schema ../ot-generate-sql-schema.cfg.xml < ../../../models/test.xmi && \
          ./ot-generate-sql-cpp ../ot-generate-sql-cpp.cfg.xml < ../../../models/test.xmi 


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
ot-generate-sql-schema.cc: ../sql-schema.xmitoolgen.xml Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@

ot-generate-sql-cpp.cc: ../sql-cpp.xmitoolgen.xml Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@

