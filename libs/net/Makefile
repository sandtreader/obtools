#==========================================================================
# Makefile for ObTools::Net
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = socket.o address.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-socket

clean: 
	-@rm -f *.o *.a *~ test-socket

test:	test-socket
	./test-socket

#Test harnesses:
test-socket: test-socket.o ot-net.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-net.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-net.h Makefile
test-socket.o: ot-net.h Makefile




