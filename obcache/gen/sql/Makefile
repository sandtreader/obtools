#==========================================================================
# Makefile for ObTools ObCache storage generator for 1-table-per-class SQL
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= ot-generate-sql-otpc
OBJS 	= _sql-otpc.o
DEPENDS = xmi regen text misc
TESTCMD = ./ot-generate-sql-otpc < ../../../models/test.xmi

ROOT    = ../../..
include $(ROOT)/build/std.mk

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
_sql-otpc.cc: ../sql-otpc.cfg.xml  \
                $(DIR-xmitoolgen)/build-release/xmitoolgen Makefile
	$(DIR-xmitoolgen)/build-release/xmitoolgen ../sql-otpc.cfg.xml > _sql-otpc.cc


