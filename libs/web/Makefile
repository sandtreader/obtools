#==========================================================================
# Makefile for ObTools::Web
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-web
HEADERS = ot-web.h
OBJS  	= mime-headers.o http-message.o url.o
TESTS 	= test-mime test-http-file test-url
TESTCMD	= ./test-mime < ../test.mime.txt
TESTCMD += && ./test-http-file < ../test.http.txt
TESTCMD += && ./test-http-file < ../test.http2.txt
TESTCMD += && ./test-url "http://foo.com:81/x/y/z?x=1&z=2\#frag"
TESTCMD += && ./test-url "/x/y/z?x=1&z=2\#frag"
DEPENDS = net xml text misc
MT-VARIANTS = 1

#Catch ourselves recursing in SINGLE mode and suppress server and use of MT
ifndef SINGLE
TESTS  	+= test-http-server
#Following test blocks run
#TESTCMD += && ./test-http-server
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



