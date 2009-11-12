#==========================================================================
# Makefile for ObTools::Access
#
# Copyright (c) 2008 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
OBJS 	= checker.o group.o resource.o rule.o
NAME   	= ot-access
HEADERS = ot-access.h
DEPENDS = xml log mt ssl net
SOCKET  = 1

TESTS 	= test-access
TESTCMD = ./test-access ../test-access.xml < ../test-access.txt

ROOT    = ../../
include $(ROOT)/build/std.mk



