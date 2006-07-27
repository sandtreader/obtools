# Borland makefile for crypto library
# Copyright (c) xMill Consulting Limited 2005

LIB = ot-crypto.lib
INCDEPS = ..\openssl\include;..\chan;..\net;..\mt;..\pthreads.2
#LIBDEPS = ..\..\..\..\obtools\libs\crypto\ot-crypto.lib
OBJS = des.obj "des-key.obj" pkcs5.obj rsa.obj "rsa-key.obj" sha1.obj
TESTS = test-des.exe test-des-key.exe test-pkcs5.exe test-rsa.exe test-rsa-key.exe test-sha1.exe

!include "..\..\build\borland.lib.mak"




