#==========================================================================
# Makefile for ObTools XMLMesh server
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

NAME   	= xmlmesh-server
HEADERS = server.h
OBJS 	= main.o server.o distributor.o correlator.o			\
	  transport-otmp-server.o service-publish.o
DEPENDS = xmlmesh-client xmlmesh-core xmlmesh-otmp net log mt xml text cache
CONFIGS = xmlmesh.cfg.xml
MULTI   = 1

ROOT    = ../..
include $(ROOT)/build/exe.mk


