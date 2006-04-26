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
	  chan:libs/chan			\
	  crypto:libs/crypto			\
	  file:libs/file                        \
	  hash:libs/hash:NOLIB  		\
	  init:libs/init			\
	  log:libs/log     			\
	  misc:libs/misc   			\
	  mt:libs/mt       			\
	  net:libs/net     			\
	  regen:libs/regen 			\
	  soap:libs/soap			\
	  text:libs/text   			\
	  time:libs/time			\
	  web:libs/web				\
	  xmi:libs/xmi     			\
	  xml:libs/xml     			\
	  xmlmesh-core:xmlmesh/core  		\
	  xmlmesh-otmp:xmlmesh/otmp  		\
	  xmlmesh-client:xmlmesh/client 	\
	  xmlmesh-c:xmlmesh/bindings/c  	\
	  toolgen:tools/toolgen			\
	  xmitoolgen:tools/xmitoolgen		\
	  xmltoolgen:tools/xmltoolgen

# List of shared libraries and associated directories
# Each word split by ':' into
# 1: Library name, including variant (as put into DEPENDS)
# 2: Directory of source, relative to ROOT
OT-SHLIBS = ot-general:libs/superlibs/general			\
	    ot-db-pgsql:libs/superlibs/db			\
	    ot-codegen:libs/superlibs/codegen			\
	    ot-xmlmesh:xmlmesh/superlibs/xmlmesh		

#==========================================================================
# Template for standard Obtools library
# $(1): Package name, including variant
# $(2): Directory, relative to root
# $(3): (optional) variant suffix for build directories
#     or NOLIB if no library required
define lib_template
DIR-$(1) = $$(ROOT)/$(2)
ifneq ($(3), NOLIB)
VARIANTS-$(1) = $(3)
LIBS-$(1)-release = $$(DIR-$(1))/build-release$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-debug   = $$(DIR-$(1))/build-debug$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-release = $$(DIR-$(1))/build-single-release$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-debug = $$(DIR-$(1))/build-single-debug$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-release-profiled = $$(DIR-$(1))/build-release-profiled$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-debug-profiled   = $$(DIR-$(1))/build-debug-profiled$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-release-profiled = $$(DIR-$(1))/build-single-release-profiled$(3)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-debug-profiled = $$(DIR-$(1))/build-single-debug-profiled$(3)$$(PLATFORM)/ot-$(1).a
endif
endef

# Split at spaces and pass on to lib_template as words
define split_template
$(call lib_template,$(word 1,$(1)),$(word 2,$(1)),$(word 3,$(1)))
endef

# Run template for each library, splitting on : to match template args (q.v.)
$(foreach lib,$(OT-LIBS),$(eval $(call split_template,$(subst :, ,$(lib)))))

define shlib_template
SDIR-$(1) = $$(ROOT)/$(2)
#Include directory is actually the build-release one
DIR-$(1) = $$(SDIR-$(1))/build-release$$(PLATFORM)
ifdef MINGW
#MinGW is not profiled
LIBS-$(1)-release = $$(SDIR-$(1))/build-release$$(PLATFORM)/$(1).dll.a
LIBS-$(1)-single-release = $$(SDIR-$(1))/build-single-release$$(PLATFORM)/$(1)-single.dll.a
LIBS-$(1)-debug = $$(SDIR-$(1))/build-debug$$(PLATFORM)/$(1).dll.a
LIBS-$(1)-single-debug = $$(SDIR-$(1))/build-single-debug$$(PLATFORM)/$(1)-single.dll.a
else
LIBS-$(1)-release = $$(SDIR-$(1))/build-release$$(PLATFORM)/lib$(1).so
LIBS-$(1)-single-release = $$(SDIR-$(1))/build-single-release$$(PLATFORM)/lib$(1)-single.so
LIBS-$(1)-release-profiled = $$(SDIR-$(1))/build-release-profiled$$(PLATFORM)/lib$(1)-profiled.so
LIBS-$(1)-single-release-profiled = $$(SDIR-$(1))/build-single-release-profiled$$(PLATFORM)/lib$(1)-single-profiled.so
endif
endef

# Split at spaces and pass on to lib_template as words
define shsplit_template
$(call shlib_template,$(word 1,$(1)),$(word 2,$(1)))
endef

# Run template for each library, splitting on : to match template args (q.v.)
$(foreach shlib,$(OT-SHLIBS),$(eval $(call shsplit_template,$(subst :, ,$(shlib)))))
