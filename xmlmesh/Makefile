#==========================================================================
# Makefile for all XMLBus components
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

all: 
	$(MAKE) -C otmp
	$(MAKE) -C client
	$(MAKE) -C servr

clean: 
	-@rm -f *~
	$(MAKE) -C otmp clean
	$(MAKE) -C client clean
	$(MAKE) -C server clean

test: 
	$(MAKE) -C otmp test
	$(MAKE) -C client test
	$(MAKE) -C server test



