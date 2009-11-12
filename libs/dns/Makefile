#==========================================================================
# Makefile for ObTools::DNS
#
# Copyright (c) 2008 ObTools Limited.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
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


