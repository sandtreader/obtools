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

# List of libraries and associated directories
# Each word split by ':' into
# 1: Library name, including variant (as put into DEPENDS)
# 2: Directory of source, relative to ROOT
# 3: Optional variant suffix of build directory, OR 'NOLIB' for templates
OT-LIBS = cli:libs/cli     			\
          cppt:libs/cppt   			\
          db-pgsql:libs/db:-pgsql 		\
	  cache:libs/cache:NOLIB  		\
	  log:libs/log     			\
	  misc:libs/misc   			\
          mt:libs/mt       			\
          net:libs/net     			\
          regen:libs/regen 			\
          text:libs/text   			\
          xmi:libs/xmi     			\
          xml:libs/xml     			\
          xmlmesh-core:xmlmesh/core  		\
          xmlmesh-otmp:xmlmesh/otmp  		\
          xmlmesh-client:xmlmesh/client 	\
          xmlmesh-c:xmlmesh/bindings/c  

#==========================================================================
# Template for standard Obtools library
# $(1): Package name, including variant
# $(2): Directory, relative to root
# $(3): (optional) variant suffix for build directories
#     or NOLIB if no library required
define lib_template
DIR-$(1) = $$(ROOT)/$(2)
ifneq ($(3), NOLIB)
LIBS-$(1)-release = $$(DIR-$(1))/build-release$(3)/ot-$(1).a
LIBS-$(1)-debug   = $$(DIR-$(1))/build-debug$(3)/ot-$(1).a
LIBS-$(1)-single-release = $$(DIR-$(1))/build-single-release$(3)/ot-$(1).a
LIBS-$(1)-single-debug = $$(DIR-$(1))/build-single-debug$(3)/ot-$(1).a
endif
endef

# Split at spaces and pass on to lib_template as words
define split_template
$(call lib_template,$(word 1,$(1)),$(word 2,$(1)),$(word 3,$(1)))
endef

# Run template for each library, splitting on : to match template args (q.v.)
$(foreach lib,$(OT-LIBS),$(eval $(call split_template,$(subst :, ,$(lib)))))





