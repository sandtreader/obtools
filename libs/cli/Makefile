#==========================================================================
# Makefile for ObTools::CLI
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-cli
HEADERS = ot-cli.h ot-cli-telnet.h
OBJS  	= command.o group.o registry.o cli.o telnet.o
DEPENDS = text net mt
TESTS 	= test-cli test-telnet
TESTCMD	= ./test-cli < ../test.txt
MULTI    = 1
SOCKET   = 1

ROOT    = ../..
include $(ROOT)/build/std.mk



