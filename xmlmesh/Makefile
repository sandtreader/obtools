#==========================================================================
# Makefile for all XMLMesh components
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

.PHONY: release clean test all

all: 
	$(MAKE) -C core
	$(MAKE) -C otmp
	$(MAKE) -C client
	$(MAKE) -C server

clean: 
	-@rm -f *~
	$(MAKE) -C core clean
	$(MAKE) -C otmp clean
	$(MAKE) -C client clean
	$(MAKE) -C server clean

test: 
	$(MAKE) -C core test
	$(MAKE) -C otmp test
	$(MAKE) -C client test
	$(MAKE) -C server test

release: 
	$(MAKE) -C core RELEASEDIR=../release/ release
	$(MAKE) -C otmp RELEASEDIR=../release/ release
	$(MAKE) -C client RELEASEDIR=../release/ release
	$(MAKE) -C server RELEASEDIR=../release/ release
	$(MAKE) -C release

