#==========================================================================
# Makefile for ObTools::MT
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

OBJS = thread.o pool.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-thread test-queue test-pool test-mutex test-rmutex

clean: 
	-@rm -f *.o *.a *~ test-thread test-queue test-pool test-mutex test-rmutex

test:	test-thread test-queue test-pool test-mutex test-rmutex
	./test-thread 10
	./test-queue

release: ot-mt.a
	cp ot-mt.a $(RELEASEDIR) 
	cp ot-mt.h $(RELEASEDIR) 

#Test harnesses:
test-thread: test-thread.o ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-queue: test-queue.o ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-pool: test-pool.o ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-mutex: test-mutex.o ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

test-rmutex: test-rmutex.o ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Core library
ot-mt.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-mt.h Makefile
test-thread.o: ot-mt.h Makefile
test-queue.o: ot-mt.h Makefile
test-pool.o: ot-mt.h Makefile
test-mutex.o: ot-mt.h Makefile
test-rmutex.o: ot-mt.h Makefile




