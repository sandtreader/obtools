#==========================================================================
# Makefile for all ObTools libraries
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

SUBS = text file xml xmi cppt regen mt net chan misc time log cache \
       db db-pgsql db-mysql                                         \
       cli init crypto ssl ssl-openssl web soap hash tube           \
       access dns daemon superlibs

#Different for cross compile
SUBS_MINGW = text xml xmi cppt regen mt net chan misc time log cache \
             cli init crypto ssl ssl-openssl file web soap hash tube \
             access dns superlibs
SUBS_MIPS = $(SUBS_MINGW)

ROOT = ..
include ../build/dir.mk


