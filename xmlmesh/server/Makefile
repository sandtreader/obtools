#==========================================================================
# Makefile for ObTools XMLMesh server
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = 	main.o server.o distributor.o correlator.o			\
	transport-otmp-server.o						\
	service-publish.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/net/ -I../../libs/log 
CPPFLAGS += -I../../libs/xml -I../../libs/text/ -I../../libs/cache/
CPPFLAGS += -I../core -I../otmp -I../client

all: xmlmesh-server

clean: 
	-@rm -f *.o *.a *~ xmlmesh-server

release: xmlmesh-server
	cp xmlmesh-server $(RELEASEDIR) 

#Test harnesses:
xmlmesh-server:	$(OBJS)    					        \
		../client/ot-xmlmesh-client.a				\
		../otmp/ot-xmlmesh-otmp.a				\
		../core/ot-xmlmesh-core.a				\
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a	\
		../../libs/xml/ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Objects
$(OBJS): server.h Makefile

