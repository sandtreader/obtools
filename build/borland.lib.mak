# Hand-carved Makefile for libraries under Borland make
# Include from library directory with LIB, OBJS and TESTS set

# Copyright (c) xMill Consulting Limited 2005

AR = tlib
CPPFLAGS = -P 

# Pseudo targets
all:	$(LIB)

clean:
	-@del /q *.obj *.lib *.exe *.tds *.map *~

test:  $(TESTS)
	&$**

# Test EXEs
$(TESTS): $(LIB)

.obj.exe: 
	$(CC) $< $(LIB)

# Library
$(LIB):  $(OBJS) 
    $(AR) /u "$@" $?

.cc.obj:
	$(CC) $(CPPFLAGS) -c $<

