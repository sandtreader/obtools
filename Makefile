#==========================================================================
# Makefile for all ObTools code
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

all: 
	$(MAKE) -C libs
	$(MAKE) -C tools

clean: 
	$(MAKE) -C libs clean
	$(MAKE) -C tools clean

test: 
	$(MAKE) -C libs test
	$(MAKE) -C tools test


