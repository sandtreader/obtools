# Hand-carved Makefile for libraries under Borland make
# Include from library directory with LIB, OBJS and TESTS set

# Copyright (c) xMill Consulting Limited 2005

AR = tlib

#Flags:
# -P:      Always assume C++
# -w-inl:  Turn off "[code] in function is not expanded inline" warnings
#            (a.k.a., "this compiler generates bad code for common idioms")

CPPFLAGS = -P -w-inl

!ifdef INCDEPS
INCS = -I$(INCDEPS)
!endif

# Pseudo targets
all:	$(LIB)

clean:
	-@del /q *.obj *.lib *.exe *.tds *.map *~

!ifdef TESTS
test:  $(TESTS)
	&$**

# Test EXEs
$(TESTS): $(LIB)
!endif

.obj.exe: 
	$(CC) $< $(LIB) $(LIBDEPS)

# Library
$(LIB):  $(OBJS) 
    $(AR) /u "$@" $?

.cc.obj:
	$(CC) $(CPPFLAGS) -c $(INCS) $<

