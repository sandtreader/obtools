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
# MULTI:     Set if multithreading is required

# This recurses on itself with target 'exe' and then also defines:

# DEBUG:     Build debug version

#==========================================================================
# Verify eval works
$(eval EVALWORKS=1)
ifndef EVALWORKS
$(error eval is broken - you need GNU make 3.80 or greater)
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

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
LIB-VARIANT = -debug
else
LIB-VARIANT = -release
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

# Main target: recurse to self to build variants
all:	build-release build-debug 

# Top-level clean target
clean: 
	-@rm -rf build.*

# Top-level test target - uses debug version
test:	build-debug
	$(MAKE) -C build.debug -f ../Makefile ROOT=../$(ROOT) runtests

# Release target - uses release version
release: build-release
	cp build.release/$(NAME) $(RELEASEDIR) 
ifdef CONFIGS
	cp $(CONFIGS) $(RELEASEDIR) 
endif

# Build targets
build-release:
	-@mkdir -p build.release
	$(MAKE) -C build.release -f ../Makefile ROOT=../$(ROOT) exe

build-debug:
	-@mkdir -p build.debug
	$(MAKE) -C build.debug -f ../Makefile DEBUG=1 ROOT=../$(ROOT) exe

# Per-build target - sub-make comes in here
exe: $(NAME)

runtests:
	$(TESTCMD)

#Executable
$(NAME): $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ $(EXTRALIBS)

#Dependencies
$(OBJS): $(HEADERS) Makefile




