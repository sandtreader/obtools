#==========================================================================
# Makefile for ObTools::Text
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

OBJS = ws.o pattern.o case.o 

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-ws test-pattern test-case

clean: 
	-@rm -f *.o *.a *~ test-ws test-pattern test-case

test:	test-ws test-pattern test-case
	./test-ws
	./test-pattern
	./test-case

release: ot-text.a
	cp ot-text.a $(RELEASEDIR) 
	cp ot-text.h $(RELEASEDIR) 

#Test harnesses:
test-ws: test-ws.o ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-pattern: test-pattern.o ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-case: test-case.o ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-text.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-text.h Makefile
test-ws.o: ot-text.h Makefile
test-pattern.o: ot-text.h Makefile
test-case.o: ot-text.h Makefile




