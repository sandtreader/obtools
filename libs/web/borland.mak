# Borland makefile for web library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-web.lib
OBJS = mime.obj
TESTS = test-mime.exe
TESTCMD = test-mime < test.mime.txt
CPPDEFS = -D_SINGLE

!include "..\..\build\borland.lib.mak"




