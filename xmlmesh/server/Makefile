#==========================================================================
# Makefile for ObTools XMLBus server
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = main.o server.o distributor.o transport-otmp.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/net/ -I../../libs/log 
CPPFLAGS += -I../../libs/xml -I../../libs/text/
CPPFLAGS += -I../core -I../otmp -I../client

all: xmlbus-server

clean: 
	-@rm -f *.o *.a *~ xmlbus-server

#Test harnesses:
xmlbus-server:	$(OBJS)    					        \
		../client/ot-xmlbus-client.a				\
		../otmp/ot-xmlbus-otmp.a				\
		../core/ot-xmlbus-core.a				\
	     	../../libs/net/ot-net.a ../../libs/mt/ot-mt.a 		\
		../../libs/log/ot-log.a ../../libs/text/ot-text.a	\
		../../libs/xml/ot-xml.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Objects
$(OBJS): server.h Makefile

