#==========================================================================
# Makefile for ObTools::Text
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = ws.o pattern.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-ws test-pattern

clean: 
	-@rm -f *.o *.a *~ test-ws test-pattern

test:	test-ws test-pattern
	./test-ws
	./test-pattern

#Test harnesses:
test-ws: test-ws.o ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-pattern: test-pattern.o ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-text.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-text.h Makefile
test-ws.o: ot-text.h Makefile
test-pattern.o: ot-text.h Makefile




