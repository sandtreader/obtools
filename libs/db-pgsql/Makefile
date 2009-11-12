#==========================================================================
# Makefile for ObTools::DB - Postgres driver
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
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





