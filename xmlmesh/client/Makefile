#==========================================================================
# Makefile for ObTools XMLBus client library
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = client.o transport-otmp.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/net/ -I../../libs/log 
CPPFLAGS += -I../../libs/xml -I../core -I../otmp

all: test-client

clean: 
	-@rm -f *.o *.a *~ test-client 

test:	test-client 
	./test-client localhost

#Test harnesses:
test-client:	test-client.o ot-xmlbus-client.a	                \
		../otmp/ot-xmlbus-otmp.a				\
		../core/ot-xmlbus-core.a				\
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a	\
		../../libs/xml/ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Client library
ot-xmlbus-client.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmlbus-client.h ../otmp/ot-xmlbus-otmp.h Makefile
test-client.o: ot-xmlbus-client.h Makefile

