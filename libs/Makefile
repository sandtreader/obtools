#==========================================================================
# Makefile for all ObTools libraries
# PS-SL-3-1 version
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

SUBS = text file xml mt net chan misc time log cache \
       init crypto ssl ssl-openssl web msg soap hash tube ring  \
       access dns daemon gather 

#Different for cross compile
SUBS_MINGW = text file xml xmi cppt regen mt net chan misc time log cache \
             cli init crypto ssl ssl-openssl web msg soap hash tube ring \
             access dns gather 
SUBS_MIPS = $(SUBS_MINGW)
SUBS_SH4 = $(SUBS_MINGW)

ROOT = ..
include ../build/dir.mk


