#==========================================================================
# Makefile for ObTools::Log
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
#==========================================================================

OBJS = distributor.o filter.o logstream.o channel.o logger.o

ifdef DEBUG
CPPFLAGS += -g -D OBTOOLS_LOG_MAX_LEVEL=LEVEL_DUMP
LDFLAGS += -g
endif

CPPFLAGS += -I../mt -I../text

all: test-log 

clean: 
	-@rm -f *.o *.a *~ test-log

test:	test-log 
	./test-log

release: ot-log.a
	cp ot-log.a $(RELEASEDIR) 
	cp ot-log.h $(RELEASEDIR) 

#Test harness:
test-log: test-log.o ot-log.a ../text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#Core library
ot-log.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-log.h Makefile
test-log.o: ot-log.h Makefile




