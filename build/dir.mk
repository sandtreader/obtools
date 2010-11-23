#==========================================================================
# Makefile for a directory - recurses to subdirectories
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

.PHONY: release clean test all docfile subdoc

all:	$(patsubst %,%-all,$(SUBS))

# Use standard SUBS if not overridden
ifndef SUBS_MINGW
SUBS_MINGW = $(SUBS)
endif

mingw:	$(patsubst %,%-mingw,$(SUBS_MINGW))

ifndef SUBS_MIPS
SUBS_MIPS = $(SUBS)
endif

mips:	$(patsubst %,%-mips,$(SUBS_MIPS))

ifndef SUBS_SH4
SUBS_SH4 = $(SUBS)
endif

sh4:	$(patsubst %,%-sh4,$(SUBS_SH4))

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
$(1)-mingw: 	; $$(MAKE) -C $(1) mingw
$(1)-mips: 	; $$(MAKE) -C $(1) mips
$(1)-sh4: 	; $$(MAKE) -C $(1) sh4
$(1)-profile: 	; $$(MAKE) -C $(1) profile
$(1)-clean: 	; $$(MAKE) -C $(1) clean
$(1)-test: 	; $$(MAKE) -C $(1) test
$(1)-release: 	; $$(MAKE) -C $(1) RELEASEDIR=../$(RELEASEDIR) release
$(1)-docfile: 	; $$(MAKE) -C $(1) docfile
endef

$(foreach sub,$(SUBS),$(eval $(call sub_template,$(sub))))


