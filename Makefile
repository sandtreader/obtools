#==========================================================================
# Makefile for all ObTools code
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

.PHONY: all clean test release

all: 
	$(MAKE) -C libs
	$(MAKE) -C tools
	$(MAKE) -C xmlmesh

clean: 
	-@rm -f *~
	$(MAKE) -C libs clean
	$(MAKE) -C tools clean
	$(MAKE) -C xmlmesh clean

test: 
	$(MAKE) -C libs test
	$(MAKE) -C tools test
	$(MAKE) -C xmlmesh test

release:
	$(MAKE) -C libs release
	$(MAKE) -C xmlmesh release