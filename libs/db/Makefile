#==========================================================================
# Makefile for ObTools::DB
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

# !!! Handle other variants?
NAME   	= ot-db-pgsql
HEADERS = ot-db.h ot-db-pgsql.h
OBJS  	= connection.o row.o db-pgsql.o
DEPENDS = log text
TESTS 	= test-pgsql
TESTCMD	= ./test-pgsql
MULTI   = 1
VARIANT-SINGLE = 1

#Don't use MT in SINGLE
ifndef SINGLE
DEPENDS += mt
endif

EXTINCS = /usr/include/postgresql/
EXTLIBS = /usr/lib/libpq.so

ROOT    = ../..
include $(ROOT)/build/lib.mk





