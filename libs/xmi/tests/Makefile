#==========================================================================
# Makefile for ObTools XMI reader tests
#
# Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
#==========================================================================

test:	test-poseidon test-argo

test-poseidon:	test-model.xmi
	LeakCheck ../test-xmi < test-model.xmi
	leak-analyze ../test-xmi

test-argo:	test-argo_.xmi
	LeakCheck ../test-xmi < test-argo_.xmi
	leak-analyze ../test-xmi

clean: 
	-@rm -f *~ *.bak leak.out test-model.xmi test-argo_.xmi

#Extract tests from model (requires unzip to be installed)
test-model.xmi: test-model.zargo
	unzip -o test-model.zargo test-model.xmi

test-argo_.xmi: test-argo.zargo
	unzip -o test-argo.zargo test-argo_.xmi





