#==========================================================================
# Standard Makefile for any libary, executable etc.
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# Define the following before including this:

# ROOT:      Relative path to obtools/ root from including Makefile
# TYPE:      Type of thing we want to build:
#              exe    Final single executable
#              exes   Final multiple executable, each with equivalent .o
#              lib    Library
#              dlmod  Dynamically loaded module (with static libraries inside)
# NAME:      Name of the executable(s)/library/module
# VERSION:   Version number of the shared superlibrary
# OBJS:      List of local objects to build in the exe/lib/dlmod
# HEADERS:   Internal header dependencies / released headers for libs
# DEPENDS:   List of ObTools libraries we depend on
# EXTINCS:   List of external include directories
# EXTLIBS:   List of external libraries 
# TESTS:     List of test executables to build
# TESTCMD:   Test command
# CONFIGS:   Configs and other files copied to release
# VARIANTS:  List of build variants (see below)
# VARIANT-xx: Flags for each variant
# DIRTY:     Extra things to delete on 'make clean' (as well as build- dirs)
# CLEANCMD:  Extra clean command on 'make clean'
# DOCFILE:   File (relative to ROOT) to append documentation to
# SOCKET:    Set if socket support (e.g. winsock) required

# This recurses on itself with target 'targets'

# Standard variant flags:
# RELEASE:   Build release version
# DEBUG:     Build debug version
# MULTI:     Build multi-threaded version
# SINGLE:    Build single-threaded version
# MINGW:     Build MinGW version (cross-compiled from Linux)
# (implicit: If no cross version, build native)

# If VARIANTS is undefined, defaults to standard release/debug set, plus
# single/multi versions if MT-VARIANTS is set, multi being the default

# If CROSS-COMPILE is set (e.g. by 'make cross'), build cross-compiled 
# versions - currently only MINGW.

# If DEBIAN-NAME is defined, builds a Debian .deb package only in release
# target, according to its type, using the following defines:
#   DEBIAN-NAME Name of package and associated scripts
#   DEBIAN-VER  Package version
#   DEBIAN-REV  Package revision
# Provide local Makedeb script and DEBIAN files directory in source dir

#==========================================================================
# Verify eval works
$(eval EVALWORKS=1)
ifndef EVALWORKS
$(error eval is broken - you need GNU make 3.80 or greater)
endif

#Check for cross-compilation
ifdef CROSS-COMPILE
# - Future - check for which

#MinGW versions
ifdef MT-VARIANTS
 ifndef VARIANTS
VARIANTS = release-mingw debug-mingw single-release-mingw single-debug-mingw
 endif
VARIANT-release-mingw		= MINGW RELEASE MULTI
VARIANT-debug-mingw		= MINGW DEBUG MULTI
VARIANT-single-release-mingw	= MINGW RELEASE SINGLE
VARIANT-single-debug-mingw	= MINGW DEBUG SINGLE
else
 ifndef VARIANTS
VARIANTS = debug-mingw release-mingw
 endif
VARIANT-debug-mingw		= MINGW DEBUG
VARIANT-release-mingw		= MINGW RELEASE
endif

else #!CROSS-COMPILE
# Default native build
ifdef MT-VARIANTS
 ifndef VARIANTS
VARIANTS = release debug single-release single-debug 
 endif
VARIANT-release			= RELEASE MULTI 
VARIANT-debug			= DEBUG MULTI
VARIANT-single-release		= RELEASE SINGLE 
VARIANT-single-debug		= DEBUG SINGLE
else
 ifndef VARIANTS
VARIANTS = debug release
 endif
VARIANT-debug		= DEBUG
VARIANT-release		= RELEASE
endif

endif #!CROSS-COMPILE

#Compiler override for MINGW build
ifdef MINGW
CC = i586-mingw32msvc-cc
CXX = i586-mingw32msvc-g++
LD = i586-mingw32msvc-ld
AR = i586-mingw32msvc-ar
EXE-SUFFIX = .exe
CPPFLAGS += -DMINGW
PLATFORM = -mingw
ifdef SOCKET
EXTRALIBS += -lwsock32
endif
endif

# Get locations
include $(ROOT)/build/locations.mk

#Work out targets - libraries
ifeq ($(TYPE), lib)
ifdef OBJS   # If no objects, no library is built
LIB          = $(NAME).a
RELEASABLE   = $(LIB)
RELEASE-NAME = $(LIB)
endif

TARGETS = $(LIB)

ifdef DEBUG         # Only build tests in DEBUG version
#Expand tests to include suffix, if set
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif

ifdef RELEASE
ifndef MINGW
CPPFLAGS += -fpic
endif
endif
endif

#Targets for executable
ifeq ($(TYPE), exe)
TARGETS = $(NAME)$(EXE-SUFFIX)
RELEASABLE = $(NAME)
RELEASE-NAME = $(NAME)
endif

#Targets for multiple executables
ifeq ($(TYPE), exes)
TARGETS = $(patsubst %,%$(EXE-SUFFIX),$(NAME))
RELEASABLE = $(NAME)
#Blank allows copy to work below
RELEASE-NAME = 
endif

#Targets for dlmod
ifeq ($(TYPE), dlmod)
TARGETS = $(NAME).so
RELEASABLE = $(NAME).so
RELEASE-NAME = $(NAME).so
CPPFLAGS += -fpic
endif

#Targets for superlib
ifeq ($(TYPE), superlib)
VERSIONM = $(word 1,$(subst ., ,$(VERSION)))

#Make sure library name is changed to reflect singleness, because directory
#location is lost in dependencies
ifdef SINGLE
SOLINK = lib$(NAME)-single.so
else
SOLINK = lib$(NAME).so
endif

SOLIB = $(SOLINK).$(VERSION)
SONAME = $(SOLINK).$(VERSIONM)
TARGETS = $(SOLIB)
RELEASABLE = $(SOLIB)
RELEASE-NAME = $(SOLIB)
RELEASE-LINK = $(SONAME)
endif

#Set standard flags
CPPFLAGS += -W -Wall

# Check if we want multithreading and/or debugging
ifdef MULTI
CPPFLAGS += -D_REENTRANT

#MinGW uses the win32-pthread library
ifdef MINGW
EXTRALIBS += -lpthreadGC2
else
EXTRALIBS += -lpthread
endif
endif

ifdef SINGLE
CPPFLAGS += -D_SINGLE
LIB-SINGLEP = -single
ifeq ($(TYPE), lib)
RELEASE-NAME = $(NAME)-single.a
endif
ifeq ($(TYPE), superlib)
RELEASE-NAME = lib$(NAME)-single.so.$(VERSION)
RELEASE-LINK = lib$(NAME)-single.so.$(VERSIONM)
endif
endif

ifdef DEBUG
CPPFLAGS += -g -DOBTOOLS_LOG_MAX_LEVEL=5 -DDEBUG
LDFLAGS += -g
LIB-DEBUGP = -debug
else
LIB-DEBUGP = -release
endif

ifdef RELEASE
#Moderate optimisation for release
CPPFLAGS += -O2
endif

ifdef DEBIAN-VARIANT
DEBIAN-VARIANT := -$(DEBIAN-VARIANT)
endif

#Sort out dependencies
define dep_template
#Expand DIR-xxx for each dependency
CPPFLAGS += -I$(DIR-$(1))

#Add library dependency for tests/exe/dlmod
LIBS += $(LIBS-$(1)$(LIB-SINGLEP)$(LIB-DEBUGP))
endef

$(foreach dep,$(DEPENDS),$(eval $(call dep_template,$(dep))))

# Sort out header propagation for superlibs
define header_template
SOHEADERS += $(DIR-$(1))/*.h
endef

# ifeq...endif commented out due to 'eval' bug in make...
#ifeq ($(TYPE), superlib)
$(foreach dep,$(DEPENDS),$(eval $(call header_template,$(dep))))
#endif

#Add external libraries and includes
CPPFLAGS += $(patsubst %,-I%,$(EXTINCS))
EXTRALIBS += $(EXTLIBS)

# Pseudo targets
.PHONY: all targets clean test release doc

# Account for moving down into build directories
vpath % ..

# Main target: build all variants
all:	$(patsubst %,build-%,$(VARIANTS))

# Cross compile: recurse with CROSS-COMPILE set
cross:
	$(MAKE) CROSS-COMPILE=1

# Top-level clean target
clean:
	-@rm -rf build-* *~ $(DIRTY)
	$(CLEANCMD)

# Top-level test target - run tests on all variants
test:	$(patsubst %,test-%,$(VARIANTS))

# Release target - get all variants to release (optionally)
# and copy configs - assumed to be invariant
release: $(patsubst %,release-%,$(VARIANTS))
ifdef CONFIGS
	cp $(CONFIGS) $(RELEASEDIR)
endif

# Documentation target - simply append the local obdoc.xml file (if it exists)
# the DOCFILE (relative to ROOT)
docfile:
	-@cat obdoc.xml >> $(ROOT)/$(DOCFILE)

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
endif
ifeq ($(TYPE), lib)
	cp $(patsubst %,../%,$(HEADERS)) ../$(RELEASEDIR)
endif
ifeq ($(TYPE), superlib)
	cd ../$(RELEASEDIR); ln -fs $(RELEASE-NAME) $(RELEASE-LINK)
endif
ifdef DEBIAN-NAME
	../Makedeb $(DEBIAN-NAME) $(DEBIAN-VERSION) $(DEBIAN-REVISION) $(DEBIAN-VARIANT)
endif
endif
endif

#Test harnesses:
define test_template
$(1)$$(EXE-SUFFIX): $(1).o $$(LIB) $$(LIBS)
	$$(CC) $$(LDFLAGS) -o $$@ $$^ -lstdc++ $$(EXTRALIBS)
TESTOBJS += $(1).o
endef

$(foreach test,$(TESTS),$(eval $(call test_template,$(test))))

#Executable
ifeq ($(TYPE), exe)
$(NAME)$(EXE-SUFFIX): $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ $(EXTRALIBS)
endif

#Multiple executables
define exe_template
#This weird order with ifeq inside the template gets round an apparent
#limitation of make - you can't call templates inside an ifeq, or it
#complains (wrongly) of missing endifs
ifeq ($(TYPE), exes)
$(1)$$(EXE-SUFFIX): $(1).o $$(LIBS)
	$$(CC) $$(LDFLAGS) -o $$@ $$^ -lstdc++ $$(EXTRALIBS)
EXEOBJS += $(1).o
endif
endef

$(foreach exe,$(NAME),$(eval $(call exe_template,$(exe))))

#Library
ifeq ($(TYPE), lib)
$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)
endif

#DL Mod
ifeq ($(TYPE), dlmod)
$(NAME).so: $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -shared -rdynamic -o $@ $^ -lstdc++ $(EXTRALIBS)
endif

#Superlib
ifeq ($(TYPE), superlib)
$(SOLIB): $(LIBS)
	cp $(SOHEADERS) .
	$(CC) $(LDFLAGS) -shared -o $@ -Wl,-soname,$(SONAME) -Wl,-whole-archive \
        $(LIBS) -Wl,-no-whole-archive -lstdc++
	ln -fs $@ $(SOLINK)
endif

#Dependencies
$(OBJS): $(patsubst %,../%,$(HEADERS)) Makefile
ifdef TESTOBJS
$(TESTOBJS): $(patsubst %,../%,$(HEADERS)) Makefile
endif
ifdef EXEOBJS
$(EXEOBJS): $(patsubst %,../%,$(HEADERS)) Makefile
endif



