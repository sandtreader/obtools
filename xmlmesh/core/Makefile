#==========================================================================
# Makefile for ObTools XMLBus core library
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = message.o 

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../../libs/mt/ -I../../libs/xml 
CPPFLAGS += -I../../libs/net/ -I../../libs/log 

all: ot-xmlbus-core.a

clean: 
	-@rm -f *.o *.a *~ test-client 

#Core library
ot-xmlbus-core.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-xmlbus.h Makefile

