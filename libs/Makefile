#==========================================================================
# Makefile for all ObTools libraries
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

SUBS = text xml xmi cppt regen mt net chan misc time log cache   \
       db db-pgsql db-mysql                                      \
       cli init crypto ssl web soap file hash tube access dns    \
       superlibs

#Different for cross compile
SUBS_MINGW = text xml xmi cppt regen mt net chan misc time log cache \
             cli init crypto ssl web soap file hash tube ssl access dns \
             superlibs
SUBS_MIPS = $(SUBS_MINGW)

ROOT = ..
include ../build/dir.mk


