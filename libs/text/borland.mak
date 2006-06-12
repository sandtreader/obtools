# Borland makefile for text library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-text.lib
OBJS = case.obj pattern.obj subst.obj ws.obj convert.obj base64.obj
TESTS = test-case.exe test-pattern.exe test-subst.exe test-ws.exe test-convert.exe test-base64.exe

!include "..\..\build\borland.lib.mak"




