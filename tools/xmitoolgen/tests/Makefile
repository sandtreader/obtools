#==========================================================================
# Makefile for ObTools XMI Tool Generator test
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

ifdef DEBUG
CXXFLAGS += -g
LDFLAGS += -g
endif

all: testtool

clean: 
	-@rm -rf *.o *.a *~ testtool _*.cc TEST

#Test generation
test:	testtool test.xmi
	./testtool < test.xmi 
#	cat out-Foo.html

#Test tool
testtool: testtool.o ../../../libs/xmi/ot-xmi.a ../../../libs/xml/ot-xml.a ../../../libs/regen/ot-regen.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Objects
testtool.o: _testtool.cc Makefile
	$(CXX) $(CXXFLAGS) -c -I.. -I../../../libs/xml/ -I../../../libs/xmi/ -I../../../libs/regen/ -o $@ $<

#Template code
_testtool.cc: testtool.cfg.xml ../xmitoolgen-cc Makefile
	../xmitoolgen-cc testtool.cfg.xml > _testtool.cc


