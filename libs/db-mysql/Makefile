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
ifdef OSX
EXTINCS = /usr/local/mysql/include/
LDFLAGS += -L/usr/local/mysql/lib
else
ifdef CENTOS
HOSTTYPE=$(shell uname -m)
ifdef HOSTTYPE
ifeq ($(HOSTTYPE), x86_64)
LDFLAGS += -L/usr/lib64/mysql
else
LDFLAGS += -L/usr/lib/mysql
endif
endif
EXTINCS = /usr/include/mysql
else
EXTINCS = /usr/include/mysql
endif
endif

EXTLIBS = -lmysqlclient_r -lz
TESTS 	= test-mysql
TESTCMD	= ./test-mysql

ROOT    = ../..
include $(ROOT)/build/std.mk





