#==========================================================================
# Makefile for ObTools General super library
#
# Copyright (c) 2005-2007 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME	= ot-general2
VERSION	= 2.4.0
CONTAINS = xml misc text net log init chan time web file crypto hash soap \
	   mt cache cli tube ssl access dns
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
