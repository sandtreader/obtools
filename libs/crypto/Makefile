#==========================================================================
# Makefile for ObTools::Crypto
#
# Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-crypto
HEADERS = ot-crypto.h
OBJS  	= pkcs5.o des-key.o
TESTS 	= test-pkcs5 test-des-key
DEPENDS = misc
TESTCMD	= ./test-pkcs5 && ./test-des-key
EXTLIBS = /usr/lib/libcrypto.so
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

