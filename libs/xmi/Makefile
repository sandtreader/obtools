#==========================================================================
# Makefile for ObTools XMI reader
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= lib
NAME   	= ot-xmi
HEADERS = ot-xmi.h ot-uml.h
OBJS	= assocend.o association.o attribute.o class.o classifier.o      \
	  element.o enum.o feature.o genelem.o general.o modelelem.o     \
	  operation.o package.o parameter.o reader.o types.o
DEPENDS = xml
TESTS 	= test-xmi
TESTCMD	= $(MAKE) -C ../tests test
CLEANCMD = $(MAKE) -C tests clean

ROOT    = ../..
include $(ROOT)/build/std.mk




