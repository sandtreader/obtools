# Borland makefile for net library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-net.lib
OBJS = socket.obj address.obj client.obj stream.obj winsock.obj
TESTS = test-client.exe
TESTCMD = test-client localhost
CPPDEFS = -D_SINGLE

!include "..\..\build\borland.lib.mak"




