#==========================================================================
# Makefile for ObTools::DB - generic database support
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME    = ot-db
HEADERS = ot-db.h
OBJS  	= connection.o row.o transaction.o
DEPENDS = log text
MT-VARIANTS = 1

#Don't use MT in SINGLE, only provide pool in MT
ifndef SINGLE
DEPENDS += mt
OBJS += pool.o
endif

ROOT    = ../..
include $(ROOT)/build/std.mk





