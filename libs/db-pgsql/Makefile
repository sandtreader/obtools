#==========================================================================
# Makefile for ObTools::DB - Postgres driver
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

NAME	= ot-db-pgsql
TYPE	= lib
HEADERS = ot-db-pgsql.h
OBJS  	= db-pgsql.o
DEPENDS = log db text 
EXTINCS = /usr/include/postgresql/
EXTLIBS = -lpq
TESTS 	= test-pgsql
TESTCMD	= ./test-pgsql
MT-VARIANTS = 1

#Don't use MT in SINGLE
ifndef SINGLE
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk





