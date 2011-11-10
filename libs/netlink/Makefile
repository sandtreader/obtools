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
EXTLIBS = -lnl
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

