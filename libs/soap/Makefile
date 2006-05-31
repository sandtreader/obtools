#==========================================================================
# Makefile for ObTools::SOAP
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-soap
HEADERS = ot-soap.h
OBJS  	= message.o fault.o parser.o http-client.o
DEPENDS = web net log xml misc text 
TESTS 	= test-soap test-http-client
TESTCMD	= ./test-soap < ../tests/soap1-2.xml \
       && ./test-http-client http://terraservice.net/terraService.asmx \
             < ../tests/place.soap.xml
MT-VARIANTS = 1

ifndef SINGLE
OBJS += url-handler.o
TESTS += test-url-handler
DEPENDS += mt 
endif

ROOT    = ../..
include $(ROOT)/build/std.mk

