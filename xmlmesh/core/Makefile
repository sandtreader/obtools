#==========================================================================
# Makefile for ObTools XMLMesh core library
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

OBJS = message.o m-error.o m-subscription.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/xml 
CPPFLAGS += -I../../libs/net/ -I../../libs/log 

all: ot-xmlmesh-core.a

clean: 
	-@rm -f *.o *.a *~ test-client 

release: ot-xmlmesh-core.a
	cp ot-xmlmesh-core.a $(RELEASEDIR) 
	cp ot-xmlmesh.h $(RELEASEDIR) 

#Core library
ot-xmlmesh-core.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmlmesh.h Makefile

