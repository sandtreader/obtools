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
	-@rm -f *~ release/
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

release: clean
	mkdir -p release/
	$(MAKE) -C xml   RELEASEDIR=../release/ release
	$(MAKE) -C xmi   RELEASEDIR=../release/ release
	$(MAKE) -C cppt  RELEASEDIR=../release/ release
	$(MAKE) -C regen RELEASEDIR=../release/ release
	$(MAKE) -C text  RELEASEDIR=../release/ release
	$(MAKE) -C mt    RELEASEDIR=../release/ release
	$(MAKE) -C net   RELEASEDIR=../release/ release
	$(MAKE) -C log   RELEASEDIR=../release/ release



