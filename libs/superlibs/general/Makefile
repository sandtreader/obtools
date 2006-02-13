#==========================================================================
# Makefile for ObTools General super library
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef MINGW
TYPE	= dll
else
TYPE	= superlib
endif
NAME	= ot-general
VERSION	= 1.5.0
DEPENDS = xml misc text net log init chan time web file crypto
MT-VARIANTS = 1
SOCKET  = 1
EXTLIBS = -lcrypto

#Only build release versions - debug is not PIC
#Don't override in MinGW
ifndef CROSS-COMPILE
VARIANTS = release single-release
endif

ifdef MINGW
# Include libeay32 import library
EXTLIBS = -leay32
else
EXTLIBS = -lcrypto
endif

DEBIAN-NAME    = libot-general
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ifdef SINGLE
DEBIAN-VARIANT = single
endif

# Supress MT, CLI & CACHE libraries if single
ifndef SINGLE
DEPENDS += mt cache cli
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk
