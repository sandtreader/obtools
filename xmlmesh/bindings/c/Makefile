#==========================================================================
# Makefile for ObTools XMLMesh C binding
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

OBJS = xmlmesh-c.o
OTLIBS = ../../../libs
CPPFLAGS += -I../../client -I../../core -I../../otmp \
	    -I$(OTLIBS)/net -I$(OTLIBS)/mt -I$(OTLIBS)/xml -I$(OTLIBS)/log

all: ot-xmlmesh-c.a

clean: 
	-@rm -f *.o *.a *~ test-client 

test:	test-client
	./test-client localhost

#Test harness
test-client:	test-client.o ot-xmlmesh-c.a		                \
		../../client/ot-xmlmesh-client.a			\
		../../otmp/ot-xmlmesh-otmp.a				\
		../../core/ot-xmlmesh-core.a				\
	     	$(OTLIBS)/net/ot-net.a $(OTLIBS)/mt/ot-mt.a 		\
		$(OTLIBS)/log/ot-log.a $(OTLIBS)/text/ot-text.a	\
		$(OTLIBS)/xml/ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Library
ot-xmlmesh-c.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): Makefile

