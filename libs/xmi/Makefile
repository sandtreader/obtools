#==========================================================================
# Makefile for ObTools XMI reader
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = reader.o uml.o
CPPFLAGS += -g -I../xml
LDFLAGS += -g 

all: test-xmi

clean: 
	-@rm -f *.o *.a *~ test-xmi leak.out

test:	test-xmi
	LeakCheck ./test-xmi < tests/test1-2.xmi
	leak-analyze ./test-xmi

#Test harness:
test-xmi: test-xmi.o ot-xmi.a ../xml/ot-xml.a 
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#XMI library
ot-xmi.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmi.h ot-uml.h ../xml/ot-xml.h Makefile
test-xmi.o: ot-xmi.h ot-uml.h Makefile




