#==========================================================================
# Locations of ObTools packages
#
# Makefile format giving locations for each ObTools library that might
# appear in a DEPENDS line
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# Pass in ROOT as relative path to obtools/ root

# ObTools libraries - get all lower-case directories (ignoring CVS)
OT-LIBS = $(notdir $(wildcard $(ROOT)/libs/[a-z]*))

# Template for standard ObTools library
define lib_template
DIR-$(1) = $$(ROOT)/libs/$(1)
INCS-$(1) = $$(DIR-$(1))/ot-$(1).h
LIBS-$(1)-release = $$(DIR-$(1))/build.release/ot-$(1).a
LIBS-$(1)-debug   = $$(DIR-$(1))/build.debug/ot-$(1).a
LIBS-$(1)-single-release = $$(DIR-$(1))/build.single.release/ot-$(1).a
LIBS-$(1)-single-debug = $$(DIR-$(1))/build.single.debug/ot-$(1).a
endef

# Run template for each library
$(foreach lib,$(OT-LIBS),$(eval $(call lib_template,$(lib))))


