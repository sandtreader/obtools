#==========================================================================
# Makefile for ObTools ReGen
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

release: ot-regen.a
	cp ot-regen.a $(RELEASEDIR) 
	cp ot-regen.h $(RELEASEDIR) 

#CLI app
regen: regen.o ot-regen.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#ReGen library
ot-regen.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-regen.h Makefile
regen.o: ot-regen.h Makefile









