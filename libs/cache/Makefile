#==========================================================================
# Makefile for ObTools::Cache
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../mt

all: test-timeout-use test-timeout-age test-pointer

clean: 
	-@rm -f *.o *.a *~ test-timeout-use test-timeout-age test-pointer

test:	test-timeout-use test-timeout-age test-pointer
	./test-pointer

release: 
	cp ot-cache.h $(RELEASEDIR) 

#Test harnesses:
test-timeout-use: test-timeout-use.o ../mt/ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-timeout-age: test-timeout-age.o ../mt/ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-pointer: test-pointer.o ../mt/ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Objects
test-timeout-use.o: ot-cache.h Makefile
test-timeout-age.o: ot-cache.h Makefile
test-pointer.o: ot-cache.h Makefile




