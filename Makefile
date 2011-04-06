#==========================================================================
# Makefile for all ObTools code
# PS-GL-2-6-MAINT version
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

.PHONY: all clean test release doc

all: 
	$(MAKE) -C libs
	$(MAKE) -C xmlmesh

mingw: 
	$(MAKE) -C libs mingw
	$(MAKE) -C xmlmesh mingw

mips: 
	$(MAKE) -C libs mips
	$(MAKE) -C xmlmesh mips

profile: 
	$(MAKE) -C libs profile
	$(MAKE) -C xmlmesh profile

clean: 
	-@rm -f *~
	$(MAKE) -C libs clean
	$(MAKE) -C xmlmesh clean

test: 
	$(MAKE) -C libs test
	$(MAKE) -C xmlmesh test

release:
	$(MAKE) -C libs release
	$(MAKE) -C xmlmesh release


