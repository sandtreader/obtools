#==========================================================================
# Makefile for ObTools::SOAP
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-soap
HEADERS = ot-soap.h
OBJS  	= message.o fault.o parser.o http-client.o url-handler.o
DEPENDS = web net log xml misc text mt 
TESTS 	= test-soap test-http-client test-url-handler
TESTCMD	= ./test-soap < ../tests/soap1-2.xml \
       && ./test-http-client http://terraservice.net/terraService.asmx \
             < ../tests/place.soap.xml
SOCKET = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

