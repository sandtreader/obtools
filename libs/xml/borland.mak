# Borland makefile for XML library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-xml.lib
OBJS = element.obj parser.obj xpath.obj config.obj
INCDEPS = ..\fix
LIBDEPS = ..\fix\ot-fix.lib

TESTS = test-xml.exe test-xpath.exe test-config.exe
TESTCMD = test-xml.exe < tests/simple.xml
TESTCMD2 = test-xpath.exe < tests/simple.xml
TESTCMD3 = test-config.exe

!include "..\..\build\borland.lib.mak"




