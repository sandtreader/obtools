#==========================================================================
# Makefile for ObTools XMI Tool Generator
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

ifdef DEBUG
CPPFLAGS += -g 
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/cppt/ -I../../libs/xml -I../../libs/text

.PHONY: test all clean 

all: xmitoolgen-cc

test:	xmitoolgen-cc 
	$(MAKE) -C tests test

clean: 
	-@rm -f *.o *~ _*.cc xmitoolgen-cc
	$(MAKE) -C tests clean


#Tool generator
xmitoolgen-cc: xmitoolgen-cc.o ../../libs/cppt/ot-cppt.a \
               ../../libs/xml/ot-xml.a ../../libs/text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Objects
xmitoolgen-cc.o: ../../libs/cppt/ot-cppt.h ../../libs/xml/ot-xml.h \
	         ../../libs/text/ot-text.h Makefile




