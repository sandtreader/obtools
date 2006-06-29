# Hand-carved Makefile for libraries under Borland make
# Include from library directory with LIB, OBJS and TESTS set

# Copyright (c) xMill Consulting Limited 2005

AR = tlib
CC = bcc32

#Flags:
# -P:      Always assume C++
# -w-inl:  Turn off "[code] in function is not expanded inline" warnings
#            (a.k.a., "this compiler generates bad code for common idioms")

CPPFLAGS = -tWM -tWC -v -y -P -DBORLAND -DDEBUG -w-inl $(CPPDEFS)

!ifdef INCDEPS
INCS = -I$(INCDEPS)
!endif

# Pseudo targets
all:	$(LIB)

clean:
	-@del /q *.obj *.lib *.exe *.tds *.map *~

!ifdef TESTS
test:  $(TESTS)
!ifdef TESTCMD
	$(TESTCMD)
	$(TESTCMD2)
	$(TESTCMD3)
	$(TESTCMD4)
	$(TESTCMD5)
!else
	&$**
!endif

# Test EXEs
$(TESTS): $(LIB)
!endif

.obj.exe: 
	$(CC) $< $(LIB) $(LIBDEPS)

# Library
!ifdef LIB
$(LIB):  $(OBJS) 
    $(AR) /u "$@" $(OBJS)
# Using OBJS instead of $? here to retain double quotes.  Otherwise tlib
# treats hyphens in filenames as remove directives.
!endif

.cc.obj:
	$(CC) $(CPPFLAGS) -c $(INCS) $<

