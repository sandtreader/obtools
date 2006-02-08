#==========================================================================
# Makefile for ObTools::Crypto
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-crypto
HEADERS = ot-crypto.h
OBJS  	= pkcs5.o des-key.o des.o rsa-key.o rsa.o 
TESTS 	= test-pkcs5 test-des-key test-des test-rsa-key test-rsa
DEPENDS = misc chan net 
TESTCMD	= ./test-pkcs5 && ./test-des-key && ./test-des \
       && ./test-rsa-key && ./test-rsa
EXTLIBS = /usr/lib/libcrypto.so
MT-VARIANTS = 1

ifndef SINGLE
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk

