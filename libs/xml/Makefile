#==========================================================================
# Makefile for ObTools::XML
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

OBJS = element.o parser.o xpath.o config.o
ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-xml test-xpath test-config

clean: 
	-@rm -f *.o *.a *~ *.~* *.obj *.exe *.lib *.dll *.tds test-xml \
        test-xpath test-config leak.out 

test:	xml-test xpath-test config-test

xml-test: test-xml
	LeakCheck ./test-xml < tests/simple.xml
	leak-analyze ./test-xml

xpath-test: test-xpath
	LeakCheck ./test-xpath < tests/simple.xml
	leak-analyze ./test-xpath

config-test: test-config
	LeakCheck ./test-config 
	leak-analyze ./test-config

release: ot-xml.a
	cp ot-xml.a $(RELEASEDIR) 
	cp ot-xml.h $(RELEASEDIR) 

#Test harnesses:
test-xml: test-xml.o ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-xpath: test-xpath.o ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-config: test-config.o ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#XML library
ot-xml.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xml.h Makefile
test-xml.o: ot-xml.h Makefile
test-xpath.o: ot-xml.h Makefile
test-config.o: ot-xml.h Makefile




