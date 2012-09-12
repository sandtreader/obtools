#==========================================================================
# Standard Makefile for any libary, executable etc.
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

# Define the following before including this:

# ROOT:      Relative path to obtools/ root from including Makefile
# TYPE:      Type of thing we want to build:
#              exe    Final single executable
#              exes   Final multiple executable, each with equivalent .o
#              lib    Library
#              dlmod  Dynamically loaded module (with static libraries inside)
#              reloc  Relocatable (partially linked) object file
# NAME:      Name of the executable(s)/library/module
# VERSION:   Version number of the shared superlibrary
# OBJS:      List of local objects to build in the exe/lib/dlmod
# HEADERS:   Internal header dependencies / released headers for libs
# DEPENDS:   List of ObTools libraries we depend on
# CONTAINS:  List of libraries we contain (superlibs, dlls)
# EXTINCS:   List of external include directories
# EXTLIBS:   List of external libraries
# RESFILE:   RC file for Windows resources 
# RESDEPS:   Other dependences for resource file
# TESTS:     List of test executables to build
# TESTCMD:   Test command
# TESTLIB:   Test libraries to link for TESTS (e.g. -lgtest)
# RELOCEXTRA: Extra archives to pull in when creating relocatable
# MAINOBJ:   Use to specify main object for exclusion in exe TESTS
#            (default: main.o)
# CONFIGS:   Configs and other files copied to release
# VARIANTS:  List of build variants (see below)
# VARIANT-xx: Flags for each variant
# DIRTY:     Extra things to delete on 'make clean' (as well as build- dirs)
# CLEANCMD:  Extra clean command on 'make clean'
# DOCFILE:   File (relative to ROOT) to append documentation to
# SOCKET:    Set if socket support (e.g. winsock) required
# WXWIDGETS: Set if WxWidgets required (requires build in external/wx)
# RESOLVER:  Set if resolver required
# EXTRA-TARGETS:  Anything else you want built
# BUILD-SUBDIRS:  Subdirectories to be created of build-xxx directories
# CLEAN-SUBDIRS:  Subdirectories of source to be cleaned (defaults to above)

# This recurses on itself with target 'targets'

# Standard variant flags:
# RELEASE:   Build release version
# DEBUG:     Build debug version
# PROFILED:  Build profiled version
# MINGW:     Build MinGW version (cross-compiled from Linux)
# (implicit: If no cross version, build native)

# If VARIANTS is undefined, defaults to standard release/debug set

# If CROSS is set (e.g. by 'make mingw' etc.), builds cross-compiled
# versions, otherwise native

# If PROFILE is set (e.g. by 'make profile'), build profiled versions

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

#Spot 'cross-compile' on MAC
OSNAME=$(shell uname -s)
ifdef OSNAME
ifeq ($(OSNAME), Darwin)
ifndef CROSS
CROSS = osx
endif
endif
endif

#Spot 'cross-compile' on CentOS, RHEL5
OSNAME=$(shell uname -r|sed 's/\(.*\)\.//'|cut -c1-2)
ifdef OSNAME
ifeq ($(OSNAME), el)
ifndef CROSS
CROSS = centos
endif
endif
endif

#Spot 'cross-compile' on CentOS, RHEL5
OSNAME=$(shell uname -r|cut -d\. -f 4|cut -c1-2)
ifdef OSNAME
ifeq ($(OSNAME), el)
ifndef CROSS
CROSS = centos
endif
endif
endif



#Check for cross-compilation
ifdef CROSS

ifeq ($(CROSS), mingw)
#MinGW versions
#Don't build debug versions - debugging is difficult anyway and
#gtest etc. isn't available
ifndef VARIANTS
VARIANTS = release-mingw
endif
VARIANT-release-mingw		= MINGW RELEASE
VARIANT-SUFFIX = -mingw
endif

ifeq ($(CROSS), mips)
#MIPS (Broadcom) chipset
#Not much point in building debug versions
ifndef VARIANTS
VARIANTS = release-mips
endif
VARIANT-release-mips		= MIPS RELEASE
VARIANT-SUFFIX = -mips
endif

ifeq ($(CROSS), sh4)
#SH4 chipset
#Not much point in building debug versions
ifndef VARIANTS
VARIANTS = release-sh4
endif
VARIANT-release-sh4		= SH4 RELEASE
VARIANT-SUFFIX = -sh4
endif

ifeq ($(CROSS), osx)
#Mac OS-X
#Note:  Still called 'cross' even though this is usually built natively
#on the mac itself
ifndef VARIANTS
ifndef RELEASE-VARIANTS-ONLY
VARIANTS += debug-osx
endif
endif
ifndef RELEASE-VARIANTS
RELEASE-VARIANTS = release-osx
endif
VARIANT-debug-osx		= OSX DEBUG
VARIANT-release-osx		= OSX RELEASE
VARIANT-SUFFIX = -osx
endif

ifeq ($(CROSS), centos)
#CentOS
#Note:  Still called 'cross' even though this is usually built natively
#on CentOS
ifndef VARIANTS
ifndef RELEASE-VARIANTS-ONLY
VARIANTS += debug
endif
endif
ifndef RELEASE-VARIANTS
RELEASE-VARIANTS = release
endif
VARIANT-debug		= CENTOS DEBUG
VARIANT-release		= CENTOS RELEASE RPM
endif

else #!CROSS-COMPILE

#Check for PROFILE - native only
ifdef PROFILE
# Profiled native build
ifndef VARIANTS
 ifndef RELEASE-VARIANTS-ONLY
VARIANTS += debug-profiled 
 endif
endif
ifndef RELEASE-VARIANTS
RELEASE-VARIANTS = release-profiled
endif
VARIANT-debug-profiled		= PROFILED DEBUG
VARIANT-release-profiled	= PROFILED RELEASE

else #!PROFILE
# Default native build
ifndef VARIANTS
ifndef RELEASE-VARIANTS-ONLY
VARIANTS = debug
endif
endif

ifndef RELEASE-VARIANTS
RELEASE-VARIANTS = release
endif

VARIANT-debug		= DEBUG
VARIANT-release		= RELEASE

endif #!PROFILE
endif #!CROSS-COMPILE

#Choose base libraries
EXTRALIBS = -lstdc++ 

#Compiler override for MINGW build
ifdef MINGW 
CC = i586-mingw32msvc-cc
CXX = i586-mingw32msvc-g++
LD = i586-mingw32msvc-ld
AR = i586-mingw32msvc-ar
WINDRES = i586-mingw32msvc-windres
EXE-SUFFIX = .exe
CPPFLAGS += -DMINGW
LDFLAGS += -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc 
PLATFORM = -mingw
ifdef SOCKET
EXTRALIBS += -lwsock32
endif
ifdef RESFILE
RESOBJ = $(RESFILE).o
endif 
else
#Compiler override for MIPS build
ifdef MIPS

#!!! Best guess - Fix these!
CC = mips-linux-uclibc-cc
CXX = mips-linux-uclibc-g++
LD = mips-linux-uclibc-ld
AR = mips-linux-uclibc-ar
CPPFLAGS += -DMIPS
PLATFORM = -mips
EXTRALIBS += -lrt
#!!!

else
#Compiler override for SH4 build
ifdef SH4

CC = sh4-linux-gcc
CXX = sh4-linux-g++
LD = sh4-linux-ld
AR = sh4-linux-ar
STRIP = sh4-linux-strip
CPPFLAGS += -DSH4
PLATFORM = -sh4
EXTRALIBS += -lrt

else
#Compiler override for OS X build
ifdef OSX
CC = i686-apple-darwin9-gcc-4.0.1
CXX = i686-apple-darwin9-g++-4.0.1
LD = ld
AR = ar
PLATFORM = -osx
CPPFLAGS += -D__BSD__ -D__OSX__

else
#Compiler override for centos X build
ifeq ($(CROSS), centos)
CC = gcc
CXX = g++
EXTRALIBS += -lrt
else

#Normal native build
CC = gcc
CXX = g++
EXTRALIBS += -lrt
STRIP = strip
endif
endif
endif
endif
endif

#Set suffix of dynamic libraries and linker looping
ifdef OSX
DYNLIB=dylib
LDLOOPSTART=
LDLOOPEND=
else
DYNLIB=so
LDLOOPSTART=-Wl,-\(
LDLOOPEND=-Wl,-\)
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

ifdef DEBUG         # Only build tests in DEBUG & PROFILED versions
#Expand tests to include suffix, if set
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif
ifdef PROFILED 
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif

ifdef RELEASE
ifndef MINGW
ifndef OSX
CPPFLAGS += -fpic

endif
endif
endif
endif

# set target architecture
HOSTTYPE=$(shell uname -m)
TARGARCH  = i386
ifdef HOSTTYPE
ifeq ($(HOSTTYPE), x86_64)
ifeq ($(CROSS), centos)
TARGARCH = x86_64
else
TARGARCH  = amd64
endif
endif
ifeq ($(HOSTTYPE), armv6l)
TARGARCH = armel
endif
endif

#Targets for executable
ifeq ($(TYPE), exe)
TARGETS = $(NAME)$(EXE-SUFFIX)
RELEASABLE = $(NAME)
RELEASE-NAME = $(NAME)

ifdef DEBUG         # Only build tests in DEBUG & PROFILED versions
#Expand tests to include suffix, if set
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif
ifdef PROFILED
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif

endif

#Targets for multiple executables
ifeq ($(TYPE), exes)
TARGETS = $(patsubst %,%$(EXE-SUFFIX),$(NAME))
RELEASABLE = $(NAME)
#Blank allows copy to work below
RELEASE-NAME = 
endif

#Targets for reloc
ifeq ($(TYPE), reloc)
TARGETS = $(NAME)-reloc.o
RELEASABLE = $(NAME)-reloc.o
RELEASE-NAME = $(NAME)-reloc.o
CPPFLAGS += -fpic

ifdef DEBUG         # Only build tests in DEBUG & PROFILED versions
#Expand tests to include suffix, if set
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif
ifdef PROFILED 
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
endif

endif

#Targets for dlmod
ifeq ($(TYPE), dlmod)
TARGETS = $(NAME).$(DYNLIB)
RELEASABLE = $(NAME).$(DYNLIB)
RELEASE-NAME = $(NAME).$(DYNLIB)
CPPFLAGS += -fpic
endif

#Targets for superlib
ifeq ($(TYPE), superlib)
VERSIONM = $(word 1,$(subst ., ,$(VERSION)))

#Make sure library name is changed to reflect profiledness, 
#because directory location is lost in dependencies
ifdef PROFILED
SOLINK = lib$(NAME)-profiled.$(DYNLIB)
else
SOLINK = lib$(NAME).$(DYNLIB)
endif

ifdef OSX
SOLIB = $(basename $(SOLINK)).$(VERSION).dylib
else
SOLIB = $(SOLINK).$(VERSION)
endif

SONAME = $(SOLINK).$(VERSIONM)
TARGETS = $(SOLIB)
RELEASABLE = $(SOLIB)
RELEASE-NAME = $(SOLIB)
RELEASE-LINK = $(SONAME)
endif

#Targets for Windows DLL
ifeq ($(TYPE), dll)

DLL-NAME = $(NAME).dll
IMPLIB-NAME = $(DLL-NAME).a
TARGETS = $(DLL-NAME)
RELEASABLE = $(DLL-NAME) $(IMPLIB-NAME)
RELEASE-NAME = 

#Note: Build release version tests, too
#Expand tests to include suffix, if set
TARGETS += $(patsubst %,%$(EXE-SUFFIX),$(TESTS))
#Set LIB to build test properly
LIB = $(IMPLIB-NAME)

endif

#Set standard flags
ifndef SUPPRESS_WARNINGS
CPPFLAGS += -W -Wall
endif

# Always reentrant
CPPFLAGS += -D_REENTRANT

#MinGW uses the win32-pthread library
ifdef MINGW

#Following required for thread-safe exception handling
CPPFLAGS += -mthreads
LDFLAGS += -mthreads

EXTRALIBS += -lpthreadGC2
else
EXTRALIBS += -lpthread
endif

ifdef DEBUG
CPPFLAGS += -g -DDEBUG
LDFLAGS += -g
LIB-DEBUGP = -debug
else
LIB-DEBUGP = -release
endif

ifdef RELEASE
#Moderate optimisation for release
CPPFLAGS += -O2
endif

ifdef PROFILED
#Standard for any profile method
CPPFLAGS += -DPROFILE
LIB-PROFILEDP = -profiled

#Use HRPROF (hrprof.sourceforge.net) to get better resolution on IA64
#We build this in build/hrprof
CPPFLAGS += -finstrument-functions 
EXTRALIBS += -lhrprof

#Alternate using standard GNU gprof
#CPPFLAGS += -pg 
#LDFLAGS += -pg

endif

#WxWidgets stuff
#(Note: use build.sh in these directories to configure/make the WX libs first)
ifdef WXWIDGETS
WX-BASE = $(ROOT)/../external/wx/
WX-BUILD = $(WX-BASE)build$(LIB-DEBUGP)$(PLATFORM)
WX-CONFIG = $(WX-BUILD)/wx-config
CPPFLAGS += `$(WX-CONFIG) --cxxflags`
EXTLIBS += `$(WX-CONFIG) --libs`
endif

#Crypto stuff
ifdef CRYPTO
ifdef MINGW
EXTLIBS += -lssl32 -leay32
else
EXTLIBS += -lssl -lcrypto
endif
endif

#Botan alternative
ifdef BOTAN
BOTAN_DIR = $(ROOT)/../external/botan
EXTINCS += $(BOTAN_DIR)/build/include
EXTLIBS += $(BOTAN_DIR)/libbotan.a
endif

#Resolver libraries
ifdef RESOLVER
ifdef MINGW
EXTLIBS += -ldnsapi
else
EXTLIBS += -lresolv
endif
endif

ifdef DEBIAN-VARIANT
DEBIAN-VARIANT := -$(DEBIAN-VARIANT)
endif

#Sort out dependencies
define dep_template
#Expand DIR-xxx for each dependency
INCS += $(DIR-$(1))

#Add external dependency (not --whole-archive) for superlib/DLL
#(U = unsorted, with duplicates)
UDEPLIBS += $(LIBS-$(1)$(LIB-DEBUGP)$(LIB-PROFILEDP))

#Add header dependency
UDEPHEADERS += $(HEADER-$(1))

#Recurse to any child dependencies held by this dependency
$(foreach dep2,$(DEPENDS-$(1)),$(eval $(call dep_template,$(dep2))))
endef

$(foreach dep,$(DEPENDS),$(eval $(call dep_template,$(dep))))

# Sort deplibs and headers to avoid duplicates
DEPLIBS = $(sort $(UDEPLIBS))
DEPHEADERS = $(sort $(UDEPHEADERS))

# Additional dependencies for contained libraries
define contain_template
INCLIBS += $(LIBS-$(1)$(LIB-DEBUGP)$(LIB-PROFILEDP))
SOHEADERS += $(wildcard $(DIR-$(1))/*.h)
endef

$(foreach con,$(CONTAINS),$(eval $(call contain_template,$(con))))

#Add internal includes, duplicates removed
CPPFLAGS += $(patsubst %,-I%,$(sort $(INCS)))

#Add external libraries and includes
CPPFLAGS += $(patsubst %,-I%,$(EXTINCS))
EXTRALIBS += $(EXTLIBS)

# Pseudo targets
.PHONY: all targets clean test release doc

# Account for moving down into build directories
vpath % ..

# Main target: build all variants
all:	$(patsubst %,build-%,$(VARIANTS))

# Cross compile for Windows: recurse with CROSS=mingw
mingw:
	$(MAKE) CROSS=mingw

# Cross compile for MIPS: recurse with CROSS=mips
mips:
	$(MAKE) CROSS=mips

# Cross compile for SH4: recurse with CROSS=sh4
sh4:
	$(MAKE) CROSS=sh4

# Profile: recurse with PROFILE set
profile:
	$(MAKE) PROFILE=1

# Top-level clean target
# Default clean subdirs to same as build 
ifndef CLEAN-SUBDIRS
CLEAN-SUBDIRS = $(BUILD-SUBDIRS)
endif

clean:
	-@rm -rf build-* *~ $(DIRTY)
	$(patsubst %,$(MAKE) -C % clean;,$(CLEAN-SUBDIRS))
	$(CLEANCMD)

# Top-level test target - run tests on all variants
test:	$(patsubst %,test-%,$(VARIANTS))

# Release target - get all variants to release (optionally)
# and copy configs - assumed to be invariant
release: $(patsubst %,release-%,$(RELEASE-VARIANTS))
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
.PHONY: build-$(1) test-$(1)
build-$(1):
	-@mkdir -p build-$(1)
ifdef BUILD-SUBDIRS
	-@mkdir -p $(patsubst %,build-$(1)/%,$(BUILD-SUBDIRS))
endif
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) targets

test-$(1): build-$(1)
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) runtests
endef

$(foreach variant,$(VARIANTS),$(eval $(call build_template,$(variant))))

# Per-build target - sub-make comes in here
.PHONY: targets runtests dorelease
targets: $(TARGETS) $(EXTRA-TARGETS)

# Release targets
define release_template
#Expand RELEASE-VARIANT-xxx for each build directory, and recurse to self 
# to release
.PHONY: release-$(1)
release-$(1):
	-@mkdir -p build-$(1)
ifdef BUILD-SUBDIRS
	-@mkdir -p $(patsubst %,build-$(1)/%,$(BUILD-SUBDIRS))
endif
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) targets
	$$(MAKE) -C build-$(1) -f ../Makefile \
                 $(patsubst %,%=1,$(VARIANT-$(1))) ROOT=../$(ROOT) dorelease
endef

$(foreach variant,$(RELEASE-VARIANTS),$(eval $(call release_template,$(variant))))

# Per-build target - sub-make comes in here
.PHONY: targets runtests dorelease
targets: $(TARGETS) $(EXTRA-TARGETS)

runtests:
ifdef DEBUG
	$(TESTCMD)
endif

dorelease:
ifdef RELEASE
ifdef RELEASEDIR
ifdef RELEASABLE
	cp $(RELEASABLE) ../$(RELEASEDIR)/$(RELEASE-NAME)
ifdef RELEASE-STRIP
ifdef STRIP
	$(STRIP) ../$(RELEASEDIR)/$(RELEASE-NAME)
endif
endif
endif
ifeq ($(TYPE), lib)
	cp $(patsubst %,../%,$(HEADERS)) ../$(RELEASEDIR)
endif
ifeq ($(TYPE), superlib)
	cd ../$(RELEASEDIR); ln -fs $(RELEASE-NAME) $(RELEASE-LINK)
endif
ifdef DEBIAN-NAME
	../Makedeb $(DEBIAN-NAME) $(DEBIAN-VERSION) $(DEBIAN-REVISION) $(TARGARCH) $(DEBIAN-VARIANT)
endif
endif
endif

#Test harnesses:
ifeq ($(TYPE), exe)
ifndef MAINOBJ
MAINOBJ = main.o
endif
NOTMAINS = $(filter-out $(MAINOBJ),$(OBJS))
define test_template
$(1)$$(EXE-SUFFIX): $(1).o $$(NOTMAINS) $$(DEPLIBS) $$(DEPHEADERS) 
	$$(CC) $$(LDFLAGS) -o $$@ $(LDLOOPSTART) $(1).o  $$(NOTMAINS) $$(DEPLIBS) $(LDLOOPEND) $$(EXTRALIBS) $(TESTLIB)
TESTOBJS += $(1).o
endef
else
ifeq ($(TYPE), reloc)
define test_template
$(1)$$(EXE-SUFFIX): $(1).o $$(RELEASABLE) $$(DEPLIBS) $$(DEPHEADERS)
	$$(CC) $$(LDFLAGS) -o $$@ $(LDLOOPSTART) $(1).o -ldl $$(RELEASABLE) $$(DEPLIBS) $(LDLOOPEND) $$(EXTRALIBS) $(TESTLIB)
TESTOBJS += $(1).o
endef
else
define test_template
$(1)$$(EXE-SUFFIX): $(1).o $$(LIB) $$(DEPLIBS) $$(DEPHEADERS)
	$$(CC) $$(LDFLAGS) -o $$@ $(LDLOOPSTART) $(1).o $$(LIB) $$(DEPLIBS) $(LDLOOPEND) $$(EXTRALIBS) $(TESTLIB)
TESTOBJS += $(1).o
endef
endif
endif

$(foreach test,$(TESTS),$(eval $(call test_template,$(test))))

#Executable
ifeq ($(TYPE), exe)
$(NAME)$(EXE-SUFFIX): $(OBJS) $(RESOBJ) $(DEPLIBS) $(DEPHEADERS)
	$(CC) $(LDFLAGS) -o $@ $(LDLOOPSTART)  $(OBJS) $(RESOBJ) $(DEPLIBS) $(LDLOOPEND) $(EXTRALIBS)
endif

#Compile resources
ifdef RESFILE
$(RESOBJ): $(RESFILE) $(RESDEPS)
	$(WINDRES) -i$< -o$@ -I.. -I$(WX-BASE)/include
endif

#Multiple executables
define exe_template
#This weird order with ifeq inside the template gets round an apparent
#limitation of make - you can't call templates inside an ifeq, or it
#complains (wrongly) of missing endifs
ifeq ($(TYPE), exes)
$(1)$$(EXE-SUFFIX): $(1).o $$(DEPLIBS) $$(DEPHEADERS)
	$$(CC) $$(LDFLAGS) -o $$@ $(LDLOOPSTART) $(1).o $$(DEPLIBS) $(LDLOOPEND) $$(EXTRALIBS)
EXEOBJS += $(1).o
endif
endef

$(foreach exe,$(NAME),$(eval $(call exe_template,$(exe))))

#Library
ifeq ($(TYPE), lib)
$(LIB): $(OBJS)
	$(AR) r $@ $(OBJS)
endif

#Relocatable (partially linked) library
ifeq ($(TYPE), reloc)
$(NAME)-reloc.o: $(OBJS)
	$(CXX) $(LDFLAGS) -nostdlib -o $@ -Wl,-Ur -Wl,-\( $(DEPLIBS) $(RELOCEXTRA) $^ -Wl,-\)
endif

#DL Mod
ifeq ($(TYPE), dlmod)
$(NAME).$(DYNLIB): $(OBJS) $(DEPLIBS) $(DEPHEADERS)
	$(CC) $(LDFLAGS) -shared -rdynamic -o $@ -Wl,-\( $(OBJS) $(DEPLIBS) -Wl,-\) $(EXTRALIBS)
endif

#Superlib
ifeq ($(TYPE), superlib)
$(SOLIB): $(INCLIBS) $(SOHEADERS)
	cp $(SOHEADERS) .
ifdef OSX
	$(CC) $(LDFLAGS) -shared -nostartfiles -Wl,-dylib -o $@ -Wl,-all_load \
	$(INCLIBS) $(DEPLIBS) $(EXTRALIBS)
else
	$(CC) $(LDFLAGS) -shared -o $@ -Wl,-soname,$(SONAME) -Wl,-whole-archive \
        $(INCLIBS) -Wl,-no-whole-archive -Wl,-\( $(DEPLIBS) -Wl,-\) $(EXTRALIBS) 
endif
	ln -fs $@ $(SOLINK)
endif

#DLL
ifeq ($(TYPE), dll)
#Aarggh!
COMMA:= ,
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
EXCLUDE-LIBS = $(subst $(SPACE),$(COMMA),$(strip $(DEPLIBS) $(EXTRALIBS)))
$(DLL-NAME): $(INCLIBS) $(OBJS) $(SOHEADERS)
ifdef SOHEADERS
	cp $(SOHEADERS) .
endif
	$(CC) $(LDFLAGS) -shared -o $@ -Wl,--out-implib=$(IMPLIB-NAME)   \
	  $(OBJS)							 \
          -Wl,--whole-archive $(INCLIBS) -Wl,--no-whole-archive          \
	  -Wl,--exclude-libs,$(EXCLUDE-LIBS)                             \
          -Wl,-\( $(DEPLIBS) -Wl,-\) $(EXTRALIBS)
endif

#Dependencies
$(OBJS): $(patsubst %,../%,$(HEADERS)) Makefile $(DEPHEADERS)
ifdef TESTOBJS
$(TESTOBJS): $(patsubst %,../%,$(HEADERS)) Makefile $(DEPHEADERS)
endif
ifdef EXEOBJS
$(EXEOBJS): $(patsubst %,../%,$(HEADERS)) Makefile $(DEPHEADERS)
endif



