#==========================================================================
# Makefile for all XMLMesh components
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

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



