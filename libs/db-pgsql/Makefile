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
DEPENDS = log db text mt
ifdef OSX
EXTINCS = /usr/local/pgsql/include/
LDFLAGS += -L/usr/local/pgsql/lib
else
EXTINCS = /usr/include/postgresql/
endif

EXTLIBS = -lpq
TESTS 	= test-pgsql
TESTCMD	= ./test-pgsql

ROOT    = ../..
include $(ROOT)/build/std.mk





