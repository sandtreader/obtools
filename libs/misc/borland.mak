# Borland makefile for misc library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-misc.lib
OBJS = md5.obj proplist.obj random.obj
TESTS = test-md5.exe test-prop.exe test-random.exe

!include "..\..\build\borland.lib.mak"




