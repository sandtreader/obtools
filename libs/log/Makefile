#==========================================================================
# Makefile for ObTools::Log
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = distributor.o logstream.o channel.o logger.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../mt 

all: test-log 

clean: 
	-@rm -f *.o *.a *~ test-log

test:	test-log 
	./test-log localhost

#Test harness:
test-log: test-log.o ot-log.a 
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-log.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-log.h Makefile
test-log.o: ot-log.h Makefile




