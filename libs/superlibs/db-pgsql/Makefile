#==========================================================================
# Makefile for ObTools Postgres Database super library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= superlib
NAME   	= ot-db-pgsql
VERSION	= 1.1.0
DEPENDS = db-pgsql
MT-VARIANTS = 1

#Only build release versions - debug is not PIC
VARIANTS = release single-release

DEBIAN-NAME    = libot-db-pgsql
DEBIAN-VERSION = $(VERSION)
DEBIAN-REVISION = 1

ifdef SINGLE
DEBIAN-VARIANT = single
endif

ROOT    = ../../..
include $(ROOT)/build/std.mk


