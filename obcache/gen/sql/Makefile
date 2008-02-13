#==========================================================================
# Makefile for ObTools ObCache storage generator for 1-table-per-class SQL
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

XMITOOLGEN = $(DIR-xmitoolgen)/build-release/xmitoolgen 

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
ot-generate-sql-schema.cc: ../sql-schema.xmitoolgen.xml  \
                Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@

ot-generate-sql-cpp.cc: ../sql-cpp.xmitoolgen.xml  \
                Makefile $(XMITOOLGEN)
	$(XMITOOLGEN) $< > $@

