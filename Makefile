#==========================================================================
# Makefile for all ObTools code
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

all: 
	$(MAKE) -C libs
	$(MAKE) -C tools
	$(MAKE) -C xmlbus

clean: 
	-@rm -f *~
	$(MAKE) -C libs clean
	$(MAKE) -C tools clean
	$(MAKE) -C xmlbus clean

test: 
	$(MAKE) -C libs test
	$(MAKE) -C tools test
	$(MAKE) -C xmlbus test


