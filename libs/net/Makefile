#==========================================================================
# Makefile for ObTools::Net
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = socket.o address.o client.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-client

clean: 
	-@rm -f *.o *.a *~ test-client

test:	test-client
	./test-client localhost

#Test harnesses:
test-client: test-client.o ot-net.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-net.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-net.h Makefile
test-client.o: ot-net.h Makefile




