#==========================================================================
# Makefile for ObTools::XML
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = element.o parser.o
CPPFLAGS += -g
LDFLAGS += -g
 
all: test-xml

clean: 
	-@rm -f *.o *.a *~ *.~* *.obj *.exe *.lib *.dll *.tds test-xml leak.out

test:	test-xml
	LeakCheck ./test-xml < tests/simple.xml
	leak-analyze ./test-xml
 
#Test harness:
test-xml: test-xml.o xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#XML library
xml.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): xml.h Makefile
test-xml.o: xml.h Makefile




