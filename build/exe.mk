#==========================================================================
# Standard Makefile for an executable
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# Define the following before including this:

# ROOT:      Relative path to obtools/ root from including Makefile
# NAME:      Name of the executable
# OBJS:      List of local objects to build in the exe
# HEADERS:   Internal header dependencies
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
VARIANT-debug = DEBUG
VARIANT-release = RELEASE
endif
endif

# Get locations
include $(ROOT)/build/locations.mk

#Set standard flags
CPPFLAGS += -W #-Wall

# Check if we want multithreading and/or debugging
ifdef MULTI
CPPFLAGS += -D_REENTRANT
EXTRALIBS += -lpthread
endif

ifdef SINGLE
CPPFLAGS += -D_SINGLE
LIB-VARIANT = -single
endif

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
LIB-VARIANT += -debug
else
LIB-VARIANT += -release
endif

#Sort out dependencies
define dep_template
#Expand DIR-xxx for each dependency
CPPFLAGS += -I$(DIR-$(1))

#Add library depending on whether we're singlethreaded
LIBS += $(LIBS-$(1)$(LIB-VARIANT))
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
ifdef CONFIGS
	cp $(CONFIGS) $(RELEASEDIR) 
endif

# Build targets
define build_template
#Expand VARIANT-xxx for each build directory, and recurse to self to build,
#test and release
build-$(1):
	-@mkdir -p build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) exe

test-$(1): build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) runtests

release-$(1): build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) dorelease
endef

$(foreach variant,$(VARIANTS),$(eval $(call build_template,$(variant))))

# Per-build target - sub-make comes in here
exe: $(NAME)

runtests:
ifdef DEBUG
	$(TESTCMD)
endif

dorelease:
ifdef RELEASE
ifdef RELEASEDIR
	cp $(NAME) ../$(RELEASEDIR) 
endif
endif

#Executable
$(NAME): $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ $(EXTRALIBS)

#Dependencies
$(OBJS): $(HEADERS) Makefile




