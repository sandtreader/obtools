#==========================================================================
# Makefile for ObTools XMLMesh PHP binding
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

TYPE	= dlmod
NAME   	= php_xmlmesh
OBJS 	= php_xmlmesh.o

DEBIAN-NAME    = ot-xmlmesh-php
DEBIAN-VERSION = 1.0.0
DEBIAN-REVISION = 2

ifdef DEBUG
DEPENDS = xmlmesh-c xmlmesh-client xmlmesh-core xmlmesh-otmp \
	  net log xml soap text cache misc
else
DEPENDS	= xmlmesh-c ot-xmlmesh ot-general
endif

PHP	= /usr/include/php4
EXTINCS = $(PHP) $(PHP)/TSRM $(PHP)/main $(PHP)/Zend

#Only current works single-threaded, due to Apache fork/pthread weirdness
SINGLE = 1

ROOT    = ../../..
include $(ROOT)/build/std.mk

