#==========================================================================
# Makefile for ObTools::DB - MySQL driver
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

NAME    = ot-db-mysql
TYPE	= lib
HEADERS = ot-db-mysql.h
OBJS  	= db-mysql.o
DEPENDS = log text db
EXTINCS = /usr/include/mysql
TESTS 	= test-mysql
TESTCMD	= ./test-mysql

MT-VARIANTS = 1

#Don't use MT in SINGLE
ifdef SINGLE
EXTLIBS = -lmysqlclient -lz
else
EXTLIBS = -lmysqlclient_r -lz
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk





