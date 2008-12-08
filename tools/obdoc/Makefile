#==========================================================================
# Makefile for ObTools ObDoc Documentor
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= exe
NAME   	= obdoc-html
OBJS 	= _obdoc-html.o
DEPENDS = xml regen text misc
TESTCMD = ./obdoc-html < ../test.obdoc.xml

ROOT    = ../..
include $(ROOT)/build/std.mk

#Turn off -Wall to avoid reams of unused parameter warning
CPPFLAGS := $(subst -Wall,,$(CPPFLAGS))

#Template code
_obdoc-html.cc: ../obdoc-html.cfg.xml  \
                $(DIR-xmltoolgen)/build-release$(VARIANT-SUFFIX)/xmltoolgen Makefile
	$(DIR-xmltoolgen)/build-release$(VARIANT-SUFFIX)/xmltoolgen ../obdoc-html.cfg.xml > _obdoc-html.cc


