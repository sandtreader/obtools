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
DEPENDS = log db text mt
EXTINCS = /usr/include/mysql
EXTLIBS = -lmysqlclient_r -lz
TESTS 	= test-mysql
TESTCMD	= ./test-mysql

ROOT    = ../..
include $(ROOT)/build/std.mk





