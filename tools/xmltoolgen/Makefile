#==========================================================================
# Makefile for ObTools XML Tool Generator
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef DEBUG
CPPFLAGS += -g 
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/cppt/ -I../../libs/xml -I../../libs/text

.PHONY: test all clean 

all: xmltoolgen-cc

test:	xmltoolgen-cc 
	$(MAKE) -C tests test

clean: 
	-@rm -f *.o *~ _*.cc xmltoolgen-cc
	$(MAKE) -C tests clean


#Tool generator
xmltoolgen-cc: xmltoolgen-cc.o ../../libs/cppt/ot-cppt.a \
               ../../libs/xml/ot-xml.a ../../libs/text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Objects
xmltoolgen-cc.o: ../../libs/cppt/ot-cppt.h ../../libs/xml/ot-xml.h \
	         ../../libs/text/ot-text.h Makefile




