#==========================================================================
# Makefile for ObTools ObDoc Documentor
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
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
ifdef DEBUG
_obdoc-html.cc: ../obdoc-html.cfg.xml  \
                $(DIR-xmltoolgen)/build-debug$(VARIANT-SUFFIX)/xmltoolgen Makefile
	$(DIR-xmltoolgen)/build-debug$(VARIANT-SUFFIX)/xmltoolgen ../obdoc-html.cfg.xml > _obdoc-html.cc
endif

ifdef RELEASE
_obdoc-html.cc: ../obdoc-html.cfg.xml  \
                $(DIR-xmltoolgen)/build-release$(VARIANT-SUFFIX)/xmltoolgen Makefile
	$(DIR-xmltoolgen)/build-release$(VARIANT-SUFFIX)/xmltoolgen ../obdoc-html.cfg.xml > _obdoc-html.cc
endif


