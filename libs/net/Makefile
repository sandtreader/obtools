#==========================================================================
# Makefile for ObTools::Net
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = socket.o address.o client.o server.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../mt

all: test-client test-server

clean: 
	-@rm -f *.o *.a *~ test-client test-server

test:	test-client test-server
	./test-client localhost

#Test harnesses:
test-client: test-client.o ot-net.a 
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-server: test-server.o ot-net.a ../mt/ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Core library
ot-net.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-net.h Makefile
test-client.o: ot-net.h Makefile
test-server.o: ot-net.h Makefile




