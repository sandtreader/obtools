#==========================================================================
# Makefile for ObTools::SSL
#
# Copyright (c) 2008 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
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

