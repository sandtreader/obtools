#==========================================================================
# Makefile for a directory - recurses to subdirectories
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

.PHONY: release clean test all

all:	$(patsubst %,%-all,$(SUBS))

test: 	$(patsubst %,%-test,$(SUBS))

#Overridable for deeper structures
ifdef RELEASEDIR
NORELEASE = 1         # It belongs to the level above
else
RELEASEDIR = release
endif

release: $(patsubst %,%-release,$(SUBS))
ifndef NORELEASE
	$(MAKE) -C $(RELEASEDIR)
endif

clean:  $(patsubst %,%-clean,$(SUBS))
	-@rm -f *~ 
ifndef NORELEASE
	$(MAKE) -C release clean
endif

# Submake template to run on every lib
define sub_template
$(1)-all: 	; $$(MAKE) -C $(1)
$(1)-clean: 	; $$(MAKE) -C $(1) clean
$(1)-test: 	; $$(MAKE) -C $(1) test
$(1)-release: 	; $$(MAKE) -C $(1) RELEASEDIR=../$(RELEASEDIR) release
endef

$(foreach sub,$(SUBS),$(eval $(call sub_template,$(sub))))


