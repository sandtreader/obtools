#==========================================================================
# Makefile for ObTools::DB
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

HEADERS = ot-db.h
OBJS  	= connection.o row.o
DEPENDS = log text
VARIANTS = release-pgsql debug-pgsql single-release-pgsql single-debug-pgsql
VARIANT-release-pgsql 		= RELEASE MULTI POSTGRES
VARIANT-debug-pgsql 		= DEBUG MULTI POSTGRES
VARIANT-single-release-pgsql 	= RELEASE SINGLE POSTGRES
VARIANT-single-debug-pgsql 	= DEBUG SINGLE POSTGRES

#Don't use MT in SINGLE
ifndef SINGLE
DEPENDS += mt
endif

ifdef POSTGRES
NAME	= ot-db-pgsql
HEADERS += ot-db-pgsql.h
OBJS 	+= db-pgsql.o
EXTINCS = /usr/include/postgresql/
EXTLIBS = /usr/lib/libpq.so
TESTS 	= test-pgsql
TESTCMD	= ./test-pgsql
endif

ROOT    = ../..
include $(ROOT)/build/lib.mk





