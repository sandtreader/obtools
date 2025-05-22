# Simple makefile to build ObTools libraries only
# Use only if you are consuming the libraries in another project, particularly
# within a CI system.  The 'tup' route is way better for development!

# Targets
all: core db extra

# Library sections
COREDIRS = access action alarm aws cache chan cli control crypto daemon dns \
	   exec expr file gather gen hash huffman init json lang lex lib llm \
	   log merkle misc msg mt net ring script soap ssl ssl-openssl text \
           time tube web xml
DBDIRS = db db-mysql db-pgsql db-sqlite
EXTRADIRS = cppt gnuplot g2d netlink regen serial xmi

# Directories
BUILDDIR = build
OBJSDIR = $(BUILDDIR)/obj
INCSDIR = $(BUILDDIR)/include
LIBSDIR = $(BUILDDIR)/lib

# Tools & flags
CC = clang++
CFLAGS = --std=c++17 -pedantic -Wall -Wextra -Werror -fPIC -DPLATFORM_LINUX -O2
AR = ar
IFLAGS = -I$(INCSDIR)

# Library-specific includes
db-pgsql-IFLAGS = $(shell pkg-config --cflags-only-I libpq)
netlink-IFLAGS = $(shell pkg-config --cflags-only-I libnl-3.0) \
                 $(shell pkg-config --cflags-only-I libnl-genl-3.0)

# Build rules
$(OBJSDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS) $(IFLAGS) $($(notdir $(@D))-IFLAGS)

# Lists we accumulate in templates - note simply expanded
CORELIBS :=
DBLIBS :=
EXTRALIBS :=
INCS :=

# Template to build libraries
# $(1) - library directory name
# $(2) - section name
# Note this has nasty repeated filterout/wildcard things because local
# variables inside called functions behave *very* strangely
define library_template

# Don't build library for template-only things
ifneq ($(strip $(filter-out $(1)/test-%.cc $(1)/legacy-%.cc, $(wildcard $(1)/*.cc))),)
$(2)LIBS += $(LIBSDIR)/ot-$(1).a
$(LIBSDIR)/ot-$(1).a: $(patsubst %.cc,$(OBJSDIR)/%.o,$(filter-out $(1)/test-%.cc $(1)/legacy-%.cc, $(wildcard $(1)/*.cc)))
	@mkdir -p $(LIBSDIR)
	$(AR) crs $(LIBSDIR)/ot-$(1).a $(patsubst %.cc,$(OBJSDIR)/%.o,$(filter-out $(1)/test-%.cc $(1)/legacy-%.cc, $(wildcard $(1)/*.cc)))
endif

INCS += $(1)/ot-$(1).h

endef

# Expand for each section
$(foreach dir,$(COREDIRS),$(eval $(call library_template,$(dir),CORE)))
$(foreach dir,$(DBDIRS),$(eval $(call library_template,$(dir),DB)))
$(foreach dir,$(EXTRADIRS),$(eval $(call library_template,$(dir),EXTRA)))

core:	includes $(CORELIBS)
db:	includes $(DBLIBS)
extra:	includes $(EXTRALIBS)

includes:
	@mkdir -p $(INCSDIR)
	@cp $(INCS) $(INCSDIR)

clean:
	-@rm -rf $(BUILDDIR)

help:
	-@echo $(DBLIBS)
