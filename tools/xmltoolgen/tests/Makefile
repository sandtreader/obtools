#==========================================================================
# Makefile for ObTools XML Tool Generator test
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef DEBUG
CXXFLAGS += -g
LDFLAGS += -g
endif

all: testtool

clean: 
	-@rm -rf *.o *.a *~ testtool _*.cc TestBook

#Test generation
test:	testtool test.xml
	./testtool < test.xml 
#	cat out-Foo.html

#Test tool
testtool: testtool.o ../../../libs/xml/ot-xml.a ../../../libs/regen/ot-regen.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Objects
testtool.o: _testtool.cc Makefile
	$(CXX) $(CXXFLAGS) -c -I.. -I../../../libs/xml/ -I../../../libs/regen/ -o $@ $<

#Template code
_testtool.cc: testtool.cfg.xml ../xmltoolgen-cc Makefile
	../xmltoolgen-cc testtool.cfg.xml > _testtool.cc


