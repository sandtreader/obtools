#==========================================================================
# Makefile for ObTools ReGen
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = mfile.o master.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: regen

clean: 
	-@rm -f *.o *.a *~ regen

test:	regen tests/template.cc tests/user.cc
	./regen tests/user.cc tests/template.cc 
 
#CLI app
regen: regen.o ot-regen.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#ReGen library
ot-regen.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-regen.h Makefile
regen.o: ot-regen.h Makefile









