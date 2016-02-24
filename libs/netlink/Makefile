#==========================================================================
# Makefile for ObTools::NetLink
#
# Copyright (c) 2011 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

TYPE    = lib
NAME    = ot-netlink
HEADERS = ot-netlink.h
OBJS    = genl.o

# Check for NL3
ifneq (,$(wildcard /usr/include/libnl3/netlink/netlink.h))
EXTINCS+= /usr/include/libnl3
EXTLIBS+= -lnl-3 -lnl-genl-3
else
EXTLIBS+= -lnl
CPPFLAGS+= -DNETLINK1_COMPAT
endif

TESTS   = test-genl
TESTLIB = -lgtest
TESTCMD = ./test-genl

# netlink won't build on CentOS
ifndef CENTOS
ROOT    = ../..
include $(ROOT)/build/std.mk
else
targets:
dorelease:
endif

