#==========================================================================
# Makefile for ObTools XMLMesh PHP binding
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE	= dlmod
NAME   	= php_xmlmesh
OBJS 	= php_xmlmesh.o

DEBIAN-NAME    = ot-xmlmesh-php5
DEBIAN-VERSION = 1.2.0
DEBIAN-REVISION = 2

ifdef DEBUG
DEPENDS = xmlmesh-c xmlmesh-client xmlmesh-core xmlmesh-otmp \
	  net log xml soap web text misc
else
# Module is very dependent on the order of ot-general and ot-xmlmesh
# here, even though you'd normally assume they would be the other way round
# If reversed, apache segfaults on startup with the module loaded!
DEPENDS	= xmlmesh-c ot-general ot-xmlmesh 
endif

PHP	= /usr/include/php5
EXTINCS = $(PHP) $(PHP)/TSRM $(PHP)/main $(PHP)/Zend $(PHP)/ext ${PHP}/regex

#Only current works single-threaded, due to Apache fork/pthread weirdness
SINGLE = 1

ROOT    = ../../..
include $(ROOT)/build/std.mk

