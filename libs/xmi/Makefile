#==========================================================================
# Makefile for ObTools XMI reader
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = reader.o
CPPFLAGS += -g -I../xml
LDFLAGS += -g 

all: test-xmi

clean: 
	-@rm -f *.o *.a *~ test-xmi leak.out

test:	test-xmi
	LeakCheck ./test-xmi < tests/simple.xmi
	leak-analyze ./test-xmi

#Test harness:
test-xmi: test-xmi.o xmi.a ../xml/xml.a 
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#XMI library
xmi.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): xmi.h ../xml/xml.h Makefile
test-xmi.o: xmi.h Makefile




