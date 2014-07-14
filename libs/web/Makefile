#==========================================================================
# Makefile for ObTools::Web
#
# Copyright (c) 2005 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-web
HEADERS = ot-web.h
OBJS  	= mime-headers.o http-message.o http-client.o http-server.o url.o \
	  cache.o cookies.o
TESTS 	= test-mime test-http-file test-url test-http-client test-http-server \
	  test-cache test-cookies test-url-gtest
TESTLIB = -lgtest
TESTCMD	= ./test-mime < ../test.mime.txt
TESTCMD += && ./test-http-file < ../test.http.txt
TESTCMD += && ./test-http-file < ../test.http2.txt
TESTCMD += && ./test-url "http://user:pass@foo.com:81/x/y/z?x=1&z=2\#frag"
TESTCMD += && ./test-url "/x/y/z?x=1&z=2&space=foo+bar&percent=%25\#frag"
TESTCMD += && ./test-http-client "http://www.packetship.com"
TESTCMD += && ./test-http-client "https://extranet.packetship.com/login.php"
TESTCMD += && ./test-cookies
TESTCMD += && ./test-url-gtest

DEPENDS = xml text misc net log mt ssl ssl-openssl file xml
SOCKET  = 1
CRYPTO  = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



