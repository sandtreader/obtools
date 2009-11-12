#==========================================================================
# Makefile for ObTools::CLI
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-cli
HEADERS = ot-cli.h ot-cli-telnet.h
OBJS  	= command.o group.o registry.o cli.o telnet.o
DEPENDS = text net mt
TESTS 	= test-cli test-telnet
TESTCMD	= ./test-cli < ../test.txt
SOCKET   = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



