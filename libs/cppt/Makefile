#==========================================================================
# Makefile for CPPT
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = processor.o
CPPFLAGS += -g
LDFLAGS += -g
 
all: test-cppt

clean: 
	-@rm -f *.o *.a *~ test-cppt _temp.cc _temp a.out

test:	test-cppt tests/simple.cppt
	./test-cppt < tests/simple.cppt > _temp.cc
	cat _temp.cc
	gcc _temp.cc -o _temp -lstdc++
	./_temp
 
#Test harness:
test-cppt: test-cppt.o ot-cppt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#CPPT library
ot-cppt.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-cppt.h Makefile
test-cppt.o: ot-cppt.h Makefile




