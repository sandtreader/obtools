# Borland makefile for net library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-chan.lib
INCDEPS = ..\net;..\mt;..\pthreads.2
OBJS = "block-chan.obj" error.obj reader.obj "stream-chan.obj" "tcp-chan.obj" writer.obj
TESTS = test-block.exe test-stream.exe

!include "..\..\build\borland.lib.mak"




