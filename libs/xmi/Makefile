#==========================================================================
# Makefile for ObTools XMI reader
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS =  assocend.o association.o attribute.o class.o classifier.o      \
	element.o enum.o feature.o genelem.o general.o modelelem.o     \
	operation.o package.o parameter.o reader.o types.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../xml

all: test-xmi

clean: 
	-@rm -f *.o *.a *~ test-xmi 
	$(MAKE) -C tests clean

test:	test-xmi
	$(MAKE) -C tests test

release: ot-xmi.a
	cp ot-xmi.a $(RELEASEDIR) 
	cp ot-xmi.h $(RELEASEDIR) 
	cp ot-uml.h $(RELEASEDIR) 

#Test harness:
test-xmi: test-xmi.o ot-xmi.a ../xml/ot-xml.a 
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#XMI library
ot-xmi.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmi.h ot-uml.h ../xml/ot-xml.h Makefile
test-xmi.o: ot-xmi.h ot-uml.h Makefile





