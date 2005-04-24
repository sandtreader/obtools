#==========================================================================
# Makefile for ObTools libraries release
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

all: 
	./makedeb-general.sh
	./makedeb-db-pgsql.sh
	./makedeb-codegen.sh
	-@rm -f *.tgz *~
	tar cvfz obtools-libs.tgz --exclude=CVS --exclude=Makefile \
        --exclude=makedeb*.sh --exclude=*.deb --exclude=*.v *

clean:
	-@rm -f *.so *.so.* *.a *.h *.tgz *~ *.v *.deb
