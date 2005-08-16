#==========================================================================
# Makefile for ObTools General super library
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= superlib
NAME	= ot-general
VERSION	= 1.1.1
DEPENDS = xml misc text net log init chan time web file
MT-VARIANTS = 1

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
