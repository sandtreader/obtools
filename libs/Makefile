#==========================================================================
# Makefile for all ObTools libraries
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

.PHONY: release clean test all

all: 
	$(MAKE) -C xml
	$(MAKE) -C xmi
	$(MAKE) -C cppt
	$(MAKE) -C regen
	$(MAKE) -C text
	$(MAKE) -C mt
	$(MAKE) -C net
	$(MAKE) -C log
	$(MAKE) -C cache
	$(MAKE) -C db
	$(MAKE) -C cli
	$(MAKE) -C misc

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
	$(MAKE) -C cache clean
	$(MAKE) -C db clean
	$(MAKE) -C cli clean
	$(MAKE) -C misc clean

test: 
	$(MAKE) -C xml test
	$(MAKE) -C xmi test
	$(MAKE) -C cppt test
	$(MAKE) -C regen test
	$(MAKE) -C text test
	$(MAKE) -C mt test
	$(MAKE) -C net test
	$(MAKE) -C log test
	$(MAKE) -C cache test
	$(MAKE) -C db test
	$(MAKE) -C cli test
	$(MAKE) -C misc test

release: 
	mkdir -p release/
	$(MAKE) -C xml   RELEASEDIR=../release/ release
	$(MAKE) -C xmi   RELEASEDIR=../release/ release
	$(MAKE) -C cppt  RELEASEDIR=../release/ release
	$(MAKE) -C regen RELEASEDIR=../release/ release
	$(MAKE) -C text  RELEASEDIR=../release/ release
	$(MAKE) -C mt    RELEASEDIR=../release/ release
	$(MAKE) -C net   RELEASEDIR=../release/ release
	$(MAKE) -C log   RELEASEDIR=../release/ release
	$(MAKE) -C cache RELEASEDIR=../release/ release
	$(MAKE) -C db    RELEASEDIR=../release/ release
	$(MAKE) -C cli   RELEASEDIR=../release/ release
	$(MAKE) -C misc  RELEASEDIR=../release/ release



