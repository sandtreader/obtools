# Borland makefile for net library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-log.lib
INCDEPS = ..\mt;..\pthreads.2;..\text
OBJS = channel.obj distributor.obj filter.obj logger.obj logstream.obj
TESTS = test-log.exe

!include "..\..\build\borland.lib.mak"




