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
           single-release-pgsql-mingw single-debug-pgsql-mingw \
	   release-mysql-mingw debug-mysql-mingw \
           single-release-mysql-mingw single-debug-mysql-mingw
VARIANT-release-pgsql-mingw 		= RELEASE MULTI POSTGRES MINGW
VARIANT-debug-pgsql-mingw 		= DEBUG MULTI POSTGRES MINGW
VARIANT-single-release-pgsql-mingw 	= RELEASE SINGLE POSTGRES MINGW
VARIANT-single-debug-pgsql-mingw 	= DEBUG SINGLE POSTGRES MINGW

VARIANT-release-mysql-mingw 		= RELEASE MULTI MYSQL MINGW
VARIANT-debug-mysql-mingw 		= DEBUG MULTI MYSQL MINGW
VARIANT-single-release-mysql-mingw 	= RELEASE SINGLE MYSQL MINGW
VARIANT-single-debug-mysql-mingw 	= DEBUG SINGLE MYSQL MINGW
else
ifdef PROFILE
#Profiled native
VARIANTS = release-profiled-pgsql debug-profiled-pgsql \
           single-release-profiled-pgsql single-debug-profiled-pgsql \
	   release-profiled-mysql debug-profiled-mysql \
           single-release-profiled-mysql single-debug-profiled-mysql
VARIANT-release-profiled-pgsql 		= PROFILED RELEASE MULTI POSTGRES
VARIANT-debug-profiled-pgsql 		= PROFILED DEBUG MULTI POSTGRES
VARIANT-single-release-profiled-pgsql 	= PROFILED RELEASE SINGLE POSTGRES
VARIANT-single-debug-profiled-pgsql 	= PROFILED DEBUG SINGLE POSTGRES

VARIANT-release-profiled-mysql 		= PROFILED RELEASE MULTI MYSQL
VARIANT-debug-profiled-mysql 		= PROFILED DEBUG MULTI MYSQL
VARIANT-single-release-profiled-mysql 	= PROFILED RELEASE SINGLE MYSQL
VARIANT-single-debug-profiled-mysql 	= PROFILED DEBUG SINGLE MYSQL
else
#Standard native
VARIANTS = release-pgsql debug-pgsql single-release-pgsql single-debug-pgsql \
           release-mysql debug-mysql single-release-mysql single-debug-mysql

VARIANT-release-pgsql 		= RELEASE MULTI POSTGRES
VARIANT-debug-pgsql 		= DEBUG MULTI POSTGRES
VARIANT-single-release-pgsql 	= RELEASE SINGLE POSTGRES
VARIANT-single-debug-pgsql 	= DEBUG SINGLE POSTGRES

VARIANT-release-mysql 		= RELEASE MULTI MYSQL
VARIANT-debug-mysql 		= DEBUG MULTI MYSQL
VARIANT-single-release-mysql 	= RELEASE SINGLE MYSQL
VARIANT-single-debug-mysql 	= DEBUG SINGLE MYSQL
endif
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

ifdef MYSQL
NAME	= ot-db-mysql
HEADERS += ot-db-mysql.h
OBJS 	+= db-mysql.o
EXTINCS = /usr/include/mysql
ifdef SINGLE
EXTLIBS = -lmysqlclient -lz
else
EXTLIBS = -lmysqlclient_r -lz
endif
TESTS 	= test-mysql
TESTCMD	= ./test-mysql
endif

ROOT    = ../..
include $(ROOT)/build/std.mk





