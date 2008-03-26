#==========================================================================
# Makefile for ObTools::Web
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-web
HEADERS = ot-web.h
OBJS  	= mime-headers.o http-message.o http-client.o http-server.o url.o
TESTS 	= test-mime test-http-file test-url test-http-client test-http-server 
TESTCMD	= ./test-mime < ../test.mime.txt
TESTCMD += && ./test-http-file < ../test.http.txt
TESTCMD += && ./test-http-file < ../test.http2.txt
TESTCMD += && ./test-url "http://foo.com:81/x/y/z?x=1&z=2\#frag"
TESTCMD += && ./test-url "/x/y/z?x=1&z=2&space=foo+bar&percent=%25\#frag"
TESTCMD += && ./test-http-client "http://www.packetship.com"
TESTCMD += && ./test-http-client "https://www.packetship.com"
DEPENDS = xml text misc net log mt ssl
SOCKET  = 1

ifdef MINGW
# Include libeay32 import library ??
EXTLIBS = -leay32
else
EXTLIBS = -lssl
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



