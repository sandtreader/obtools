#==========================================================================
# Makefile for all ObTools tools
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

all: 
	$(MAKE) -C xmltools
	$(MAKE) -C xmitools

clean: 
	-@rm -f *~
	$(MAKE) -C xmltools clean
	$(MAKE) -C xmitools clean

test: 
	$(MAKE) -C xmltools test
	$(MAKE) -C xmitools test



