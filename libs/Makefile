#==========================================================================
# Makefile for all ObTools libraries
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

all: 
	$(MAKE) -C xml
	$(MAKE) -C xmi
	$(MAKE) -C cppt
	$(MAKE) -C regen
	$(MAKE) -C text
	$(MAKE) -C mt
	$(MAKE) -C net
	$(MAKE) -C log

clean: 
	-@rm -f *~
	$(MAKE) -C xml clean
	$(MAKE) -C xmi clean
	$(MAKE) -C cppt clean
	$(MAKE) -C regen clean
	$(MAKE) -C text clean
	$(MAKE) -C mt clean
	$(MAKE) -C net clean
	$(MAKE) -C log clean

test: 
	$(MAKE) -C xml test
	$(MAKE) -C xmi test
	$(MAKE) -C cppt test
	$(MAKE) -C regen test
	$(MAKE) -C text test
	$(MAKE) -C mt test
	$(MAKE) -C net test
	$(MAKE) -C log test



