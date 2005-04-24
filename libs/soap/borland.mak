# Borland makefile for SOAP library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-soap.lib
OBJS = message.obj fault.obj parser.obj 
INCDEPS = ..\xml
LIBDEPS = ..\xml\ot-xml.lib ..\fix\ot-fix.lib 

TESTS = test-soap.exe 
TESTCMD = test-soap.exe < tests\soap1-2.xml

!include "..\..\build\borland.lib.mak"




