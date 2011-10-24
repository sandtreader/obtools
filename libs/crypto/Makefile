#==========================================================================
# Makefile for ObTools::Crypto
#
# Copyright (c) 2006 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-crypto
HEADERS = ot-crypto.h
OBJS  	= pkcs5.o aes-key.o aes.o des-key.o des.o rsa-key.o rsa.o sha1.o \
          x509.o store.o library.o
TESTS 	= test-pkcs5 test-aes-key test-aes test-aes-string \
          test-des-key test-des \
          test-rsa-key test-rsa test-sha1 test-x509
DEPENDS = misc chan net xml mt text
TESTLIB = -lgtest
TESTCMD	= ./test-pkcs5 && ./test-aes-key && ./test-aes && ./test-aes-string \
       && ./test-des-key && ./test-des \
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

