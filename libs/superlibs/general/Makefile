#==========================================================================
# Makefile for ObTools General super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= superlib
NAME	= ot-general
VERSION	= 1.0.0
DEPENDS = xml misc text net log init chan time web
MT-VARIANTS = 1

DEBIAN-NAME    = libot-general
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 2

ifdef SINGLE
DEBIAN-VARIANT = single
endif

# Supress MT, CLI & CACHE libraries if single
ifndef SINGLE
DEPENDS += mt cache cli
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk
