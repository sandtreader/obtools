#==========================================================================
# Makefile for ObTools::DB
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = connection.o row.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

all: test-pgsql

clean: 
	-@rm -f *.o *.a *~ test-*

release: ot-db-pgsql.a
	cp ot-db-pgsql.a $(RELEASEDIR) 
	cp ot-db.h $(RELEASEDIR) 

test: test-pgsql
	./test-pgsql

#Test harnesses:
test-pgsql: test-pgsql.o ot-db-pgsql.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

#DB libraries
ot-db-pgsql.a: $(OBJS) db-pgsql.o
	$(AR) r $@ $^

#Objects
$(OBJS): ot-db.h Makefile
db-pgsql.o: ot-db.h ot-db-pgsql.h Makefile
test-pgsql.o: ot-db.h ot-db-pgsql.h Makefile




