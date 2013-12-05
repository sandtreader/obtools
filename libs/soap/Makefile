#==========================================================================
# Makefile for ObTools::SOAP
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-soap
HEADERS = ot-soap.h
OBJS  	= message.o fault.o parser.o http-client.o url-handler.o
DEPENDS = web net log xml misc text mt msg
TESTS 	= test-soap test-http-client test-url-handler
TESTCMD	= ./test-soap < ../tests/soap1-2.xml \
       && ./test-http-client http://testhost/location.php \
	http://testhost/ConvertPlaceToLonLatPt \
	< ../tests/place.soap.xml
SOCKET = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

