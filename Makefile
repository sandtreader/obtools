#==========================================================================
# Makefile for all ObTools code
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

.PHONY: all clean test release doc

all: 
	$(MAKE) -C libs
	$(MAKE) -C tools
	$(MAKE) -C xmlmesh
	$(MAKE) -C angel

cross: 
	$(MAKE) -C libs cross
	$(MAKE) -C xmlmesh cross

profile: 
	$(MAKE) -C libs profile
	$(MAKE) -C xmlmesh profile

clean: 
	-@rm -f *~
	-@rm -rf doc.out
	$(MAKE) -C libs clean
	$(MAKE) -C tools clean
	$(MAKE) -C xmlmesh clean
	$(MAKE) -C angel clean

test: 
	$(MAKE) -C libs test
	$(MAKE) -C tools test
	$(MAKE) -C xmlmesh test
	$(MAKE) -C angel test

release:
	$(MAKE) -C libs release
	$(MAKE) -C xmlmesh release
	$(MAKE) -C angel release

doc:	all
	-@rm -rf doc.out
	-@mkdir -p doc.out
	-@cat obdoc.xml > doc.out/combined.obdoc.xml
	$(MAKE) -C libs DOCFILE=doc.out/combined.obdoc.xml docfile
	-@cat tail.obdoc.xml >> doc.out/combined.obdoc.xml
	-@cp tools/obdoc/obdoc.css doc.out/
	( cd doc.out; ../tools/obdoc/build-release/obdoc-html < combined.obdoc.xml )

