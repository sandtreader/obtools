#==========================================================================
# Makefile for ObTools XMLBus OTMP protocol library
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = client.o message.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/net/ -I../../libs/log -I../otmp

all: test-client

clean: 
	-@rm -f *.o *.a *~ test-client 

test:	test-client 
	./test-client localhost

#Test harnesses:
test-client:	test-client.o ot-xmlbus.a	  	                \
		../otmp/ot-xmlbus-otmp.a				\
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Client library
ot-xmlbus.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmlbus.h ../otmp/ot-xmlbus-otmp.h Makefile
test-client.o: ot-xmlbus.h Makefile

