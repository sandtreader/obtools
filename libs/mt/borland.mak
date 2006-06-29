# Borland makefile for text library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-mt.lib
INCDEPS = ..\pthreads.2
LIBDEPS = ..\pthreads.2\pthreadBC2.lib
OBJS = pool.obj thread.obj
TESTS = test-mutex.exe test-pool.exe test-queue.exe test-rmutex.exe test-thread.exe test-thread-malloc.exe

!include "..\..\build\borland.lib.mak"




