#==========================================================================
# Makefile for ObTools XMI reader tests
#
# Copyright (c) 2003 Paul Clark.  All rights reserved
# This code comes with NO WARRANTY and is subject to licence agreement
#==========================================================================

test:	test-poseidon test-argo

test-poseidon:	test-model.xmi
	../build-debug/test-xmi < test-model.xmi

test-argo:	test-argo_.xmi
	../build-debug/test-xmi < test-argo_.xmi

clean: 
	-@rm -f *~ *.bak test-model.xmi test-argo_.xmi

#Extract tests from model (requires unzip to be installed)
test-model.xmi: test-model.zargo
	unzip -o test-model.zargo test-model.xmi

test-argo_.xmi: test-argo.zargo
	unzip -o test-argo.zargo test-argo_.xmi





