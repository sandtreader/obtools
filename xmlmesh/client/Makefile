#==========================================================================
# Makefile for ObTools XMLMesh client library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

OBJS = client.o mclient.o transport-otmp.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/net/ -I../../libs/log 
CPPFLAGS += -I../../libs/xml -I../../libs/text -I../core -I../otmp

all: test-client test-mclient

clean: 
	-@rm -f *.o *.a *~ test-client test-mclient 

test:	test-client 
	./test-client localhost

release: ot-xmlmesh-client.a
	cp ot-xmlmesh-client.a $(RELEASEDIR) 
	cp ot-xmlmesh-client.h $(RELEASEDIR) 
	cp ot-xmlmesh-client-otmp.h $(RELEASEDIR) 

#Test harnesses:
test-client:	test-client.o ot-xmlmesh-client.a	                \
		../otmp/ot-xmlmesh-otmp.a				\
		../core/ot-xmlmesh-core.a				\
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a	\
		../../libs/xml/ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-mclient:	test-mclient.o ot-xmlmesh-client.a	                \
		../otmp/ot-xmlmesh-otmp.a				\
		../core/ot-xmlmesh-core.a				\
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a	\
		../../libs/xml/ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Client library
ot-xmlmesh-client.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmlmesh-client.h ../otmp/ot-xmlmesh-otmp.h Makefile
test-client.o: ot-xmlmesh-client.h ot-xmlmesh-client-otmp.h Makefile
test-mclient.o: ot-xmlmesh-client.h ot-xmlmesh-client-otmp.h Makefile

