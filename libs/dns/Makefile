#==========================================================================
# Makefile for ObTools::DNS
#
# Copyright (c) 2008 ObTools Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-dns
HEADERS = ot-dns.h 
OBJS  	= resolver.o

SOCKET = 1
RESOLVER = 1

DEPENDS = text log chan misc
TESTS 	= test-resolver
TESTCMD	= ./test-resolver carrot.org

ROOT    = ../..
include $(ROOT)/build/std.mk


