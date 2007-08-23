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
EXTINCS = /usr/include/postgresql/
EXTLIBS = -lpq
TESTS 	= test-pgsql
TESTCMD	= ./test-pgsql

ROOT    = ../..
include $(ROOT)/build/std.mk





