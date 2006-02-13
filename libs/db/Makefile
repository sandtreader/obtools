#==========================================================================
# Makefile for ObTools::DB
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
HEADERS = ot-db.h
OBJS  	= connection.o row.o transaction.o
DEPENDS = log text

ifdef CROSS-COMPILE
VARIANTS = release-pgsql-mingw debug-pgsql-mingw \
           single-release-pgsql-mingw single-debug-pgsql-mingw
VARIANT-release-pgsql-mingw 		= RELEASE MULTI POSTGRES MINGW
VARIANT-debug-pgsql-mingw 		= DEBUG MULTI POSTGRES MINGW
VARIANT-single-release-pgsql-mingw 	= RELEASE SINGLE POSTGRES MINGW
VARIANT-single-debug-pgsql-mingw 	= DEBUG SINGLE POSTGRES MINGW
else
VARIANTS = release-pgsql debug-pgsql single-release-pgsql single-debug-pgsql
VARIANT-release-pgsql 		= RELEASE MULTI POSTGRES
VARIANT-debug-pgsql 		= DEBUG MULTI POSTGRES
VARIANT-single-release-pgsql 	= RELEASE SINGLE POSTGRES
VARIANT-single-debug-pgsql 	= DEBUG SINGLE POSTGRES
endif

#Don't use MT in SINGLE
ifndef SINGLE
DEPENDS += mt
endif

ifdef POSTGRES
NAME	= ot-db-pgsql
HEADERS += ot-db-pgsql.h
OBJS 	+= db-pgsql.o
EXTINCS = /usr/include/postgresql/
EXTLIBS = -lpq
TESTS 	= test-pgsql
TESTCMD	= ./test-pgsql
endif

ROOT    = ../..
include $(ROOT)/build/std.mk





