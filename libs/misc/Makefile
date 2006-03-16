#==========================================================================
# Makefile for ObTools::Misc
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= lib
NAME   	= ot-misc
HEADERS = ot-misc.h
OBJS  	= md5.o crc.o proplist.o random.o dumper.o
TESTS 	= test-md5 test-crc test-prop test-random test-dump 
TESTCMD	= ./test-md5 && ./test-crc && ./test-prop \
       && ./test-random && ./test-dump
MT-VARIANTS = 1

ROOT    = ../..
include $(ROOT)/build/std.mk

