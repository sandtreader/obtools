#==========================================================================
# Makefile for ObTools::Web
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-web
HEADERS = ot-web.h
OBJS  	= mime-headers.o http-message.o
TESTS 	= test-mime test-http-file 
TESTCMD	= ./test-mime < ../test.mime.txt
TESTCMD += && ./test-http-file < ../test.http.txt
DEPENDS = net xml text
MT-VARIANTS = 1

#Catch ourselves recursing in SINGLE mode and suppress server and use of MT
ifndef SINGLE
TESTS  	+= test-http-server
TESTCMD += && ./test-http-server
DEPENDS += mt
endif

ROOT    = ../..
include $(ROOT)/build/std.mk



