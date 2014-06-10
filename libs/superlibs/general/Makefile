#==========================================================================
# Makefile for ObTools General super library
#
# Copyright (c) 2005-2007 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME	= ot-general2
VERSION	= 2.5
CONTAINS = xml misc text net log init chan time web file crypto hash soap \
	   mt cache cli tube ssl access dns msg gather ring
SOCKET  = 1
CRYPTO  = 1
RESOLVER = 1

#Only build release versions - debug is not PIC
RELEASE-VARIANTS-ONLY = 1

DEBIAN-NAME    = libot-general2
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 3

ROOT    = ../../..
include $(ROOT)/build/std.mk
