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

# ObTools full libraries
OT-LIBS = cli cppt db log misc mt net regen text xmi xml

# ObTools include-only (template) libraries
OT-INCS = cache

# XMLMesh libraries
XMLMESH-LIBS = core otmp client

#==========================================================================
# Template for standard ObTools library
define lib_template
DIR-$(1) = $$(ROOT)/libs/$(1)
INCS-$(1) = $$(DIR-$(1))/ot-$(1).h
LIBS-$(1)-release = $$(DIR-$(1))/build-release/ot-$(1).a
LIBS-$(1)-debug   = $$(DIR-$(1))/build-debug/ot-$(1).a
LIBS-$(1)-single-release = $$(DIR-$(1))/build-single-release/ot-$(1).a
LIBS-$(1)-single-debug = $$(DIR-$(1))/build-single-debug/ot-$(1).a
endef

# Run template for each library
$(foreach lib,$(OT-LIBS),$(eval $(call lib_template,$(lib))))

# Template for standard ObTools include
define inc_template
DIR-$(1) = $$(ROOT)/libs/$(1)
INCS-$(1) = $$(DIR-$(1))/ot-$(1).h
endef

# Run template for each include
$(foreach inc,$(OT-INCS),$(eval $(call inc_template,$(inc))))

# Template for XMLMesh library
define xmlmesh_template
DIR-xmlmesh-$(1) = $$(ROOT)/xmlmesh/$(1)
LIBS-xmlmesh-$(1)-release = $$(DIR-xmlmesh-$(1))/build-release/ot-xmlmesh-$(1).a
LIBS-xmlmesh-$(1)-debug   = $$(DIR-xmlmesh-$(1))/build-debug/ot-xmlmesh-$(1).a
LIBS-xmlmesh-$(1)-single-release = $$(DIR-xmlmesh-$(1))/build-single-release/ot-xmlmesh-$(1).a
LIBS-xmlmesh-$(1)-single-debug = $$(DIR-xmlmesh-$(1))/build-single-debug/ot-xmlmesh-$(1).a
endef

# Run template for each XMLMesh part
$(foreach lib,$(XMLMESH-LIBS),$(eval $(call xmlmesh_template,$(lib))))



