#==========================================================================
# Makefile for ObTools ReGen
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = mfile.o master.o regenbuf.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: regen

clean: 
	-@rm -f *.o *.a *~ regen *.gen

test:	regen tests/example.h
	./regen foo.gen tests/example.h
	./regen foo.gen tests/example.h 
 
#CLI app
regen: regen.o ot-regen.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#ReGen library
ot-regen.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-regen.h Makefile
regen.o: ot-regen.h Makefile









