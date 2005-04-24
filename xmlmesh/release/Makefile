#==========================================================================
# Makefile for ObTools XMLMesh release
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

all:
	./makedeb-xmlmesh.sh
	-@rm -f *.tgz *~
	tar cvfz obtools-xmlmesh.tgz --exclude=CVS --exclude=Makefile \
        --exclude=makedeb*.sh --exclude=*.deb --exclude=*.v *

clean:
	-@rm -f *.so *.so.* *.a *.h *.tgz *.cfg.xml \
	xmlmesh-server *.deb *.v *~





