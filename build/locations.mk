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

# List of libraries and associated directories and dependencies
# Each word split by ':' into
# 1: Library name, including variant (as put into DEPENDS)
# 2: Directory of source, relative to ROOT
# 3: Dependencies (comma separated)
# 4: Optional variant suffix of build directory, OR 'NOLIB' for templates
OT-LIBS = cli:libs/cli:text,net,mt		\
	  cppt:libs/cppt   			\
	  db:libs/db:log	 		\
	  db-mysql:libs/db-mysql:log,db		\
	  db-pgsql:libs/db-pgsql:log,db		\
	  cache:libs/cache:mt:NOLIB  		\
	  chan:libs/chan:net			\
	  crypto:libs/crypto:misc,chan		\
	  file:libs/file:text                   \
	  hash:libs/hash:-:NOLIB  		\
	  init:libs/init:xml			\
	  log:libs/log:text,time 		\
	  misc:libs/misc:xml,chan		\
	  mt:libs/mt       			\
	  net:libs/net:mt     			\
	  regen:libs/regen 			\
	  script:libs/script:log,misc,init	\
	  soap:libs/soap:web,log,misc,xml	\
	  text:libs/text   			\
	  time:libs/time			\
	  tube:libs/tube:net,log,mt,chan,misc   \
	  web:libs/web:xml,misc,log,net		\
	  xmi:libs/xmi:xml     			\
	  xml:libs/xml:text    			\
	  xmlmesh-core:xmlmesh/core:net,log,soap,web,misc   		\
	  xmlmesh-otmp:xmlmesh/otmp:net,log,text,tube			\
	  xmlmesh-client:xmlmesh/client:xmlmesh-otmp,xmlmesh-core	\
	  xmlmesh-c:xmlmesh/bindings/c  	\
	  toolgen:tools/toolgen			\
	  xmitoolgen:tools/xmitoolgen		\
	  xmltoolgen:tools/xmltoolgen           \
	  obcache-core:obcache/libs/core:cache       \
	  obcache-sql:obcache/libs/sql:obcache-core,xml,db

# List of shared libraries and associated directories
# Each word split by ':' into
# 1: Library name, including variant (as put into DEPENDS)
# 2: Directory of source, relative to ROOT
OT-SHLIBS = ot-general2:libs/superlibs/general			\
	    ot-db2:libs/superlibs/db				\
	    ot-db-pgsql2:libs/superlibs/db-pgsql		\
	    ot-db-mysql2:libs/superlibs/db-mysql		\
	    ot-codegen2:libs/superlibs/codegen			\
	    ot-xmlmesh2:xmlmesh/superlibs/xmlmesh		

#==========================================================================
# Template for standard Obtools library
# $(1): Package name, including variant
# $(2): Directory, relative to root
# $(3): (optional) dependencies, separated by comma
# $(4): (optional) variant suffix for build directories
#     or NOLIB if no library required

COMMA:= ,

define lib_template
DIR-$(1) = $$(ROOT)/$(2)
DEPENDS-$(1) = $(subst $(COMMA), ,$(strip $(3)))

ifneq ($(4), NOLIB)
VARIANTS-$(1) = $(4)
LIBS-$(1)-release = $$(DIR-$(1))/build-release$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-debug   = $$(DIR-$(1))/build-debug$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-release = $$(DIR-$(1))/build-single-release$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-debug = $$(DIR-$(1))/build-single-debug$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-release-profiled = $$(DIR-$(1))/build-release-profiled$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-debug-profiled   = $$(DIR-$(1))/build-debug-profiled$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-release-profiled = $$(DIR-$(1))/build-single-release-profiled$(4)$$(PLATFORM)/ot-$(1).a
LIBS-$(1)-single-debug-profiled = $$(DIR-$(1))/build-single-debug-profiled$(4)$$(PLATFORM)/ot-$(1).a
endif
endef

# Split at spaces and pass on to lib_template as words
define split_template
$(call lib_template,$(word 1,$(1)),$(word 2,$(1)),$(word 3,$(1)),$(word 4,$(1)))
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
