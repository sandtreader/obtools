#==========================================================================
# Makefile for ObTools XMI Tool Generator test
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../../../libs/xml/ -I../../../libs/xmi/

all: testtool

clean: 
	-@rm -rf *.o *.a *~ testtool _*.cc TEST

#Test generation
test:	testtool test.xmi
	./testtool < test.xmi 
#	cat out-Foo.html

#Test tool
testtool: testtool.o ../../../libs/xmi/ot-xmi.a ../../../libs/xml/ot-xml.a 
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Objects
testtool.o: _testtool.cc Makefile
	$(CXX) -c -I.. -I../../../libs/xml/ -I../../../libs/xmi/ -o $@ $<

#Template code
_testtool.cc: testtool.cfg.xml ../xmitoolgen-cc Makefile
	../xmitoolgen-cc testtool.cfg.xml > _testtool.cc


