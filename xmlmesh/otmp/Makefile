#==========================================================================
# Makefile for ObTools XMLMesh OTMP protocol library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

OBJS = client.o server.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/net/ -I../../libs/log -I../shared

all: test-client test-server

clean: 
	-@rm -f *.o *.a *~ test-client test-server

test:	test-client test-server
	./test-client localhost

release: ot-xmlmesh-otmp.a
	cp ot-xmlmesh-otmp.a $(RELEASEDIR) 
	cp ot-xmlmesh-otmp.h $(RELEASEDIR) 

#Test harnesses:
test-client:	test-client.o ot-xmlmesh-otmp.a	  	                \
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-server:	test-server.o ot-xmlmesh-otmp.a	  	                \
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Client library
ot-xmlmesh-otmp.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmlmesh-otmp.h Makefile
test-client.o: ot-xmlmesh-otmp.h Makefile
test-server.o: ot-xmlmesh-otmp.h Makefile

