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
# MULTI:     Set if multithreading is required
# VARIANT-SINGLE:  Set if single-threaded version is also required

# This recurses on itself with target 'targets' and then also defines:

# DEBUG:     Build debug version
# SINGLE:    Build single-threaded version (overriding MULTI)

#==========================================================================
# Verify eval works
$(eval EVALWORKS=1)
ifndef EVALWORKS
$(error eval is broken - you need GNU make 3.80 or greater)
endif

# Get locations
include $(ROOT)/build/locations.mk

#Work out targets
ifdef OBJS   # If no objects, no library is built
LIB 	= $(NAME).a
endif

TARGETS = $(LIB)

ifdef DEBUG         # Only build tests in DEBUG version
TARGETS += $(TESTS)
endif

#Set standard flags
CPPFLAGS += -W #-Wall

# Check if we want multithreading and/or debugging
ifdef MULTI
ifndef SINGLE
CPPFLAGS += -D_REENTRANT
EXTRALIBS += -lpthread
endif
endif

ifdef SINGLE
CPPFLAGS += -D_SINGLE
SINGLEP = -single
endif

ifdef DEBUG
CPPFLAGS += -g
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

# Main target: recurse to self to build variants
DEBUG-BUILDS = build-debug
RELEASE-BUILDS = build-release

ifdef VARIANT-SINGLE
DEBUG-BUILDS += build-single-debug
RELEASE-BUILDS += build-single-release
endif

BUILDS = $(RELEASE-BUILDS) $(DEBUG-BUILDS) 

all:	$(BUILDS)

# Top-level clean target
clean: 
	-@rm -rf build.*

# Top-level test target - uses debug version
test:	build-debug
	$(MAKE) -C build.debug -f ../Makefile ROOT=../$(ROOT) runtests

# Release target - uses release version
release: $(RELEASE-BUILDS)
ifdef OBJS
	cp build.release/$(LIB) $(RELEASEDIR) 
ifdef VARIANT-SINGLE
	cp build.single.release/$(LIB) $(RELEASEDIR)/$(NAME).single.a
endif
endif
	cp $(HEADERS) $(RELEASEDIR) 

# Build targets
build-release:
	-@mkdir -p build.release
	$(MAKE) -C build.release -f ../Makefile ROOT=../$(ROOT) targets

build-debug:
	-@mkdir -p build.debug
	$(MAKE) -C build.debug -f ../Makefile DEBUG=1 ROOT=../$(ROOT) targets

build-single-release:
	-@mkdir -p build.single.release
	$(MAKE) -C build.single.release -f ../Makefile SINGLE=1 ROOT=../$(ROOT) targets

build-single-debug:
	-@mkdir -p build.single.debug
	$(MAKE) -C build.single.debug -f ../Makefile DEBUG=1 SINGLE=1 ROOT=../$(ROOT) targets

# Per-build target - sub-make comes in here
targets: $(TARGETS)

runtests:
	$(TESTCMD)

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




