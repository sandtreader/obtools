#==========================================================================
# Standard Makefile for any libary, executable etc.
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# Define the following before including this:

# ROOT:      Relative path to obtools/ root from including Makefile
# TYPE:      Type of thing we want to build:
#              exe    Final executable
#              lib    Library
#              dlmod  Dynamically loaded module (with static libraries inside)
# NAME:      Name of the executable/library/module
# OBJS:      List of local objects to build in the exe/lib/dlmod
# HEADERS:   Internal header dependencies / released headers for libs
# DEPENDS:   List of ObTools libraries we depend on
# EXTINCS:   List of external include directories
# EXTLIBS:   List of external libraries 
# TESTCMD:   Test command
# CONFIGS:   Configs and other files copied to release
# VARIANTS:  List of build variants
# VARIANT-xx: Flags for each variant

# This recurses on itself with target 'exe'

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
VARIANT-debug 	= DEBUG
VARIANT-release = RELEASE
endif
endif

# Get locations
include $(ROOT)/build/locations.mk

#Work out targets - libraries
ifeq ($(TYPE), lib)
ifdef OBJS   # If no objects, no library is built
LIB 	= $(NAME).a
RELEASABLE   = $(LIB)
RELEASE-NAME = $(LIB)
endif

TARGETS = $(LIB)
ifdef DEBUG         # Only build tests in DEBUG version
TARGETS += $(TESTS)
endif
endif

#Targets for executable
ifeq ($(TYPE), exe)
TARGETS = $(NAME)
RELEASABLE = $(NAME)
RELEASE-NAME = $(NAME)
endif

#Targets for dlmod
ifeq ($(TYPE), dlmod)
TARGETS = $(NAME).so
RELEASABLE = $(NAME).so
RELEASE-NAME = $(NAME).so
CPPFLAGS += -fpic
endif

#Set standard flags
CPPFLAGS += -W #-Wall

# Check if we want multithreading and/or debugging
ifdef MULTI
CPPFLAGS += -D_REENTRANT
EXTRALIBS += -lpthread
endif

ifdef SINGLE
CPPFLAGS += -D_SINGLE
LIB-SINGLEP = -single
ifeq ($(TYPE), lib)
RELEASE-NAME = $(NAME)-single.a
endif
endif

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
LIB-DEBUGP = -debug
else
LIB-DEBUGP = -release
endif

#Sort out dependencies
define dep_template
#Expand DIR-xxx for each dependency
CPPFLAGS += -I$(DIR-$(1))

#Add library dependency for tests/exe/dlmod
LIBS += $(LIBS-$(1)$(LIB-SINGLEP)$(LIB-DEBUGP))
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
# and copy configs - assumed to be invariant
release: $(patsubst %,release-%,$(VARIANTS))
ifdef CONFIGS
	cp $(CONFIGS) $(RELEASEDIR) 
endif

# Build targets
define build_template
#Expand VARIANT-xxx for each build directory, and recurse to self to build,
#test and release
.PHONY: build-$(1) test-$(1) release-$(1)
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

# Per-build target - sub-make comes in here
.PHONY: targets runtests dorelease
targets: $(TARGETS)

runtests:
ifdef DEBUG
	$(TESTCMD)
endif

dorelease:
ifdef RELEASE
ifdef RELEASEDIR
ifdef RELEASABLE
	cp $(RELEASABLE) ../$(RELEASEDIR)/$(RELEASE-NAME) 
ifeq ($(TYPE), lib)
	cp $(patsubst %,../%,$(HEADERS)) ../$(RELEASEDIR) 
endif
endif
endif
endif

#Test harnesses:
define test_template
$(1): $(1).o $$(LIB) $$(LIBS)
	$$(CC) $$(LDFLAGS) -o $$@ $$^ -lstdc++ $$(EXTRALIBS)
TEST_OBJS += $(1).o
endef

$(foreach test,$(TESTS),$(eval $(call test_template,$(test))))

#Executable
ifeq ($(TYPE), exe)
$(NAME): $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ $(EXTRALIBS)
endif

ifeq ($(TYPE), lib)
$(LIB): $(OBJS)
	$(AR) r $@ $^
endif

ifeq ($(TYPE), dlmod)
$(NAME).so: $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -shared -rdynamic -o $@ $^ -lstdc++ $(EXTRALIBS)
endif

#Dependencies
$(OBJS): $(HEADERS) Makefile
ifdef TESTOBJS
$(TESTOBJS): $(HEADERS) Makefile
endif



