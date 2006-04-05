#==========================================================================
# Makefile for a directory - recurses to subdirectories
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

.PHONY: release clean test all docfile subdoc

all:	$(patsubst %,%-all,$(SUBS))

cross:	$(patsubst %,%-cross,$(SUBS))

profile: $(patsubst %,%-profile,$(SUBS))

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

#Build documentation - bracketed by my own
docfile:
	-@cat obdoc.xml >> $(ROOT)/$(DOCFILE)
	$(MAKE) subdoc
	-@cat tail.obdoc.xml >> $(ROOT)/$(DOCFILE)

subdoc:	$(patsubst %,%-docfile,$(SUBS))


# Submake template to run on every lib
define sub_template
$(1)-all: 	; $$(MAKE) -C $(1)
$(1)-cross: 	; $$(MAKE) -C $(1) cross
$(1)-profile: 	; $$(MAKE) -C $(1) profile
$(1)-clean: 	; $$(MAKE) -C $(1) clean
$(1)-test: 	; $$(MAKE) -C $(1) test
$(1)-release: 	; $$(MAKE) -C $(1) RELEASEDIR=../$(RELEASEDIR) release
$(1)-docfile: 	; $$(MAKE) -C $(1) docfile
endef

$(foreach sub,$(SUBS),$(eval $(call sub_template,$(sub))))


