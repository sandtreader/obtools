# Borland makefile for XMI library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-xmi.lib
OBJS	= assocend.obj association.obj attribute.obj class.obj classifier.obj \
	  element.obj enum.obj feature.obj genelem.obj general.obj            \
          modelelem.obj operation.obj package.obj parameter.obj reader.obj    \
          types.obj
INCDEPS = ..\xml
LIBDEPS = ..\xml\ot-xml.lib ..\fix\ot-fix.lib 

TESTS = test-xmi.exe 

!include "..\..\build\borland.lib.mak"




