#==========================================================================
# Makefile for ObTools XMI reader tests
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================


test:	test-model.xmi
	LeakCheck ../test-xmi < test-model.xmi
	leak-analyze ../test-xmi

clean: 
	-@rm -f *~ *.bak leak.out test-model.xmi

#Extract test from model (requires unzip to be installed)
test-model.xmi: test-model.zargo
	unzip -o test-model.zargo test-model.xmi






