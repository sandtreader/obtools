#==========================================================================
# Makefile for all ObTools libraries
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

.PHONY: release clean test all

LIBS = xml xmi cppt regen misc text mt net log cache db cli 

all:	$(patsubst %,%-all,$(LIBS))

clean:  $(patsubst %,%-clean,$(LIBS))
	-@rm -f *~ 
	$(MAKE) -C release clean

test: 	$(patsubst %,%-test,$(LIBS))

release: $(patsubst %,%-release,$(LIBS))
	$(MAKE) -C release

# Submake template to run on every lib
define lib_template
$(1)-all: 	; $$(MAKE) -C $(1)
$(1)-clean: 	; $$(MAKE) -C $(1) clean
$(1)-test: 	; $$(MAKE) -C $(1) test
$(1)-release: 	; $$(MAKE) -C $(1) RELEASEDIR=../release release
endef

$(foreach lib,$(LIBS),$(eval $(call lib_template,$(lib))))




