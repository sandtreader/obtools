#==========================================================================
# Makefile for ObTools::CLI
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

OBJS = command.o group.o registry.o cli.o telnet.o

ifdef DEBUG
CPPFLAGS += -g
LDFLAGS += -g
endif

CPPFLAGS += -I../text -I../net -I../mt

all: test-cli test-telnet

clean: 
	-@rm -f *.o *.a *~ test-cli test-telnet

test:	test-cli
	./test-cli < test.txt

release: ot-cli.a
	cp ot-cli.a $(RELEASEDIR) 
	cp ot-cli.h $(RELEASEDIR) 
	cp ot-cli-telnet.h $(RELEASEDIR) 

#Test harnesses:
test-cli: test-cli.o ot-cli.a ../text/ot-text.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++

test-telnet: test-telnet.o ot-cli.a ../text/ot-text.a ../net/ot-net.a  \
	../mt/ot-mt.a
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lpthread

#Core library
ot-cli.a: $(OBJS)
	$(AR) r $@ $^

#Objects
$(OBJS): ot-cli.h Makefile
test-cli.o: ot-cli.h Makefile
test-telnet.o: ot-cli.h ot-cli-telnet.h Makefile




