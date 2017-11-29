VERSION=1.0.1-1~dev1
DATE=$(shell date --rfc-2822)



INSTALLPATH=$(shell ./getinstallpath.pl|sed 's/^.//')
#INSTALLPATH=$(shell getinstallpath.pl|sed 's#^/##')

release	:
	sed 's/__VERSION__/$(VERSION)/;s/__DATE__/$(DATE)/' changelog > debian/changelog
	echo "XMLMesh.pm $(INSTALLPATH)/ObTools" > debian/install
	fakeroot dpkg-buildpackage -uc -tc -A
