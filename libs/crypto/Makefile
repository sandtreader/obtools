#==========================================================================
# Makefile for ObTools::Crypto
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-crypto
HEADERS = ot-crypto.h
OBJS  	= pkcs5.o des-key.o des.o rsa-key.o rsa.o sha1.o x509.o library.o
TESTS 	= test-pkcs5 test-des-key test-des test-rsa-key test-rsa test-sha1 \
	  test-x509
DEPENDS = misc chan net xml mt text
TESTCMD	= ./test-pkcs5 && ./test-des-key && ./test-des \
       && ./test-rsa-key && ./test-rsa && ./test-sha1 \
       && ./test-x509 < test.x509.pem
SOCKET = 1

ifdef MINGW
# Include libeay32 import library
EXTLIBS = -leay32
else
EXTLIBS = -lcrypto
endif

ROOT    = ../..
include $(ROOT)/build/std.mk

