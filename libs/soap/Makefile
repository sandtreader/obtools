#==========================================================================
# Makefile for ObTools::SOAP
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-soap
HEADERS = ot-soap.h
OBJS  	= message.o fault.o
DEPENDS = xml
TESTS 	= test-soap
TESTCMD	= ./test-soap < ../tests/soap1-2.xml
#MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

