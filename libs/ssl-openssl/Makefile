#==========================================================================
# Makefile for ObTools::SSL
#
# Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-ssl-openssl
HEADERS = ot-ssl-openssl.h
OBJS  	= context.o connection.o
TESTS 	= test-client test-server
DEPENDS = mt net log crypto xml text ssl
TESTCMD	= 
SOCKET = 1
CRYPTO = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

