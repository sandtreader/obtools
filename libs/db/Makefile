#==========================================================================
# Makefile for ObTools::DB
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

POSTGRES = 1

OBJS = connection.o row.o

ifdef POSTGRES
CPPFLAGS += -I/usr/include/postgresql/
LIBS += ot-db-pgsql.a
TESTS += test-pgsql
PGTEST = ./test-pgsql
LIBPQ = /usr/lib/libpq.so
endif

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../log -I../mt

all: $(TESTS)

clean: 
	-@rm -f *.o *.a *~ $(TESTS)

release: $(LIBS)
	cp $(LIBS) $(RELEASEDIR) 
	cp ot-db.h $(RELEASEDIR) 
	cp ot-db-pgsql.h $(RELEASEDIR)

test: $(TESTS)
	$(PGTEST)

#Test harnesses:
test-pgsql: test-pgsql.o ot-db-pgsql.a ../log/ot-log.a ../mt/ot-mt.a \
		../text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ $(LIBPQ)

#DB libraries
ot-db-pgsql.a: $(OBJS) db-pgsql.o
	$(AR) r $@ $^

#Objects
$(OBJS): ot-db.h Makefile
db-pgsql.o: ot-db.h ot-db-pgsql.h Makefile
test-pgsql.o: ot-db.h ot-db-pgsql.h Makefile




