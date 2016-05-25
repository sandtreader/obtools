#==========================================================================
# Makefile for ObTools::Web
#
# Copyright (c) 2005 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-web
HEADERS = ot-web.h
OBJS  	= mime-headers.o http-message.o http-client.o http-server.o url.o \
	  cache.o cookies.o
TESTS 	= test-cookies test-url-gtest
TESTLIB = -lgtest
TESTCMD = ./test-cookies && ./test-url-gtest

DEPENDS = xml text misc net log mt ssl ssl-openssl file xml
SOCKET  = 1
CRYPTO  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



