#==========================================================================
# Makefile for all ObTools tools
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

all: 
	$(MAKE) -C xmltoolgen
	$(MAKE) -C xmitoolgen

clean: 
	-@rm -f *~
	$(MAKE) -C xmltoolgen clean
	$(MAKE) -C xmitoolgen clean

test: 
	$(MAKE) -C xmltoolgen test
	$(MAKE) -C xmitoolgen test



