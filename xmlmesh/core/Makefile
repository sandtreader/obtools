#==========================================================================
# Makefile for ObTools XMLMesh core library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-xmlmesh-core
HEADERS = ot-xmlmesh.h
OBJS    = message.o m-error.o m-subscription.o
DEPENDS = xml net log
MT-VARIANTS = 1

# Suppress MT library if single 
ifndef SINGLE
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk


