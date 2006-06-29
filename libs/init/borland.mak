# Borland makefile for net library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-init.lib
OBJS = init.obj
TESTS = test-init.exe
TESTCMD = test-init
CPPDEFS = -D_SINGLE
INCDEPS = ..\xml

!include "..\..\build\borland.lib.mak"




