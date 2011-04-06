#==========================================================================
# Makefile for all ObTools libraries
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

SUBS = text file xml mt net chan misc time log cache \
       db db-mysql                                         \
       init crypto ssl ssl-openssl web msg soap hash tube  \
       access dns daemon  

#Different for cross compile
SUBS_MINGW = text file xml mt net chan misc time log cache \
             init crypto ssl ssl-openssl web msg soap hash tube \
             access dns  
SUBS_MIPS = $(SUBS_MINGW)
SUBS_SH4 = $(SUBS_MINGW)

ROOT = ..
include ../build/dir.mk


