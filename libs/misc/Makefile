#==========================================================================
# Makefile for ObTools::Misc
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = md5.o proplist.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-md5 test-prop

clean: 
	-@rm -f *.o *.a *~ test-md5 test-prop

test:	test-md5 test-prop
	./test-md5
	./test-prop

release: ot-misc.a
	cp ot-misc.a $(RELEASEDIR) 
	cp ot-misc.h $(RELEASEDIR) 

#Test harnesses:
test-md5: test-md5.o ot-misc.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-prop: test-prop.o ot-misc.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-misc.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-misc.h Makefile
test-md5.o: ot-misc.h Makefile
test-prop.o: ot-misc.h Makefile




