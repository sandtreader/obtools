#==========================================================================
# Standard Makefile for a library and associated test harnesses
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# Define the following before including this:

# ROOT:      Relative path to obtools/ root from including Makefile
# NAME:      Name of the library, no suffix (e.g. ot-xxx)
# HEADERS:   Released headers (e.g. ot-xxx.h)
# OBJS:      List of objects to build in the library (e.g foo.o bar.o)
# DEPENDS:   List of libraries we depend on
# EXTINCS:   List of external include directories
# EXTLIBS:   List of external libraries for tests
# TESTS:     List of test harnesses (e.g. test-client)
# TESTCMD:   Test command
# VARIANTS:  List of build variants
# VARIANT-xx: Flags for each variant

# This recurses on itself with target 'targets'

# Standard variant flags:
# RELEASE:   Build release version
# DEBUG:     Build debug version
# MULTI:     Build multi-threaded version
# SINGLE:    Build single-threaded version

# If VARIANTS is undefined, defaults to standard release/debug set, plus
# single/multi versions if MT-VARIANTS is set, multi being the default
#==========================================================================
# Verify eval works
$(eval EVALWORKS=1)
ifndef EVALWORKS
$(error eval is broken - you need GNU make 3.80 or greater)
endif

# Default VARIANTS and flags if not set
ifndef VARIANTS
ifdef MT-VARIANTS
VARIANTS = release debug single-release single-debug
VARIANT-release 	= RELEASE MULTI
VARIANT-debug 		= DEBUG MULTI
VARIANT-single-release 	= RELEASE SINGLE
VARIANT-single-debug 	= DEBUG SINGLE
else
VARIANTS = debug release
VARIANT-debug = DEBUG
VARIANT-release = RELEASE
endif
endif

# Get locations
include $(ROOT)/build/locations.mk

#Work out targets
ifdef OBJS   # If no objects, no library is built
LIB 	= $(NAME).a
RELEASE-LIB = $(LIB)
endif

TARGETS = $(LIB)

ifdef DEBUG         # Only build tests in DEBUG version
TARGETS += $(TESTS)
endif

#Set standard flags
CPPFLAGS += -W #-Wall

# Check variant flags
ifdef MULTI
CPPFLAGS += -D_REENTRANT
EXTRALIBS += -lpthread
endif

ifdef SINGLE
CPPFLAGS += -D_SINGLE
SINGLEP = -single
RELEASE-LIB = $(NAME)-single.a
endif

ifdef DEBUG
CPPFLAGS += -g -D_DEBUG
LDFLAGS += -g
endif

#Sort out dependencies
define dep_template
#Expand DIR-xxx for each dependency
CPPFLAGS += -I$(DIR-$(1))

#Add test library depending on whether we're singlethreaded
TESTLIBS += $(LIBS-$(1)$(SINGLEP)-debug)
endef

$(foreach dep,$(DEPENDS),$(eval $(call dep_template,$(dep))))

#Add external libraries and includes
CPPFLAGS += $(patsubst %,-I%,$(EXTINCS))
EXTRALIBS += $(EXTLIBS)

# Pseudo targets
.PHONY: all targets clean test release

# Account for moving down into build directories
vpath % ..

# Main target: build all variants
all:	$(patsubst %,build-%,$(VARIANTS))

# Top-level clean target
clean: 
	-@rm -rf build-*

# Top-level test target - run tests on all variants
test:	$(patsubst %,test-%,$(VARIANTS))

# Release target - get all variants to release (optionally)
# and copy headers 
release: $(patsubst %,release-%,$(VARIANTS))
	cp $(HEADERS) $(RELEASEDIR) 

# Build targets
define build_template
#Expand VARIANT-xxx for each build directory, and recurse to self to build,
#test and release
build-$(1):
	-@mkdir -p build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) targets

test-$(1): build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) runtests

release-$(1): build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) dorelease
endef

$(foreach variant,$(VARIANTS),$(eval $(call build_template,$(variant))))

# Per-build targets - sub-make comes in here
targets: $(TARGETS)

runtests:
ifdef DEBUG
	$(TESTCMD)
endif

dorelease:
ifdef RELEASE
ifdef RELEASEDIR
ifdef OBJS
	cp $(LIB) ../$(RELEASEDIR)/$(RELEASE-LIB)
endif
endif
endif

#Test harnesses:
define test_template
$(1): $(1).o $$(LIB) $$(TESTLIBS)
	$$(CC) $$(LDFLAGS) -o $$@ $$^ -lstdc++ $(EXTRALIBS)
TEST_OBJS += $(1).o
endef

$(foreach test,$(TESTS),$(eval $(call test_template,$(test))))

#Library 
$(LIB): $(OBJS)
	$(AR) r $@ $^

#Dependencies
$(OBJS) $(TESTOBJS): $(HEADERS) Makefile




