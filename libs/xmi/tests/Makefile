#==========================================================================
# Makefile for ObTools XMI reader tests
#
# Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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





