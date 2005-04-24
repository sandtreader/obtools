#!/bin/sh
#==========================================================================
# Makedeb file for ObTools Code Generation libraries.
#
# Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

#This file originally by Will Colwyn <willc@westerntelecom.co.uk>
#Copyright assigned by Western Telecom Limited under 
#Software Licensing Agreement dated 13th January 2005

set -e

NAME=ot-codegen
DESC='code generation libraries'
PACKAGE=lib$NAME
SRC_DIR=`dirname $0`
VERSION=`cat $NAME.v`
RELEASE=1
ARCH=i386

# split version
OLDIFS=$IFS
IFS='\.'
read VERSIONM VERSIONm VERSIONP <<EOF
$VERSION
EOF
IFS=$OLDIFS

DEB_RUN=/tmp/${PACKAGE}.$$
DEB_DEV=/tmp/${PACKAGE}-dev.$$

mkdir -p $DEB_RUN/DEBIAN
mkdir -p $DEB_DEV/DEBIAN
mkdir -p $DEB_RUN/usr/lib/obtools
mkdir -p $DEB_DEV/usr/lib/obtools
mkdir -p $DEB_DEV/usr/include/obtools

# create control and postinst
cat > $DEB_RUN/DEBIAN/control <<EOF
Package: $PACKAGE
Version: ${VERSION}-${RELEASE}
Section: libs
Priority: optional
Architecture: $ARCH
Depends: libc6 (>= 2.3.2), libstdc++5 (>= 3.3.3)
Maintainer: xMill Consulting <info@xmill.com>
Description: ObTools shared libraries - $DESC.
EOF

cat > $DEB_DEV/DEBIAN/control <<EOF
Package: ${PACKAGE}-dev
Version: ${VERSION}-${RELEASE}
Section: libs
Priority: optional
Architecture: $ARCH
Depends: $PACKAGE (>= $VERSION-$RELEASE)
Maintainer: xMill Consulting <info@xmill.com>
Description: ObTools shared libraries - $DESC.
 Development package - static libraries and headers
EOF

cat > $DEB_RUN/DEBIAN/postinst <<EOF
#!/bin/sh

set -e

if ! grep -qs ^/usr/lib/obtools\$ /etc/ld.so.conf; then
  echo /usr/lib/obtools >> /etc/ld.so.conf
fi

if [ "\$1" = "configure" ]; then
    ldconfig
fi
EOF

cat > $DEB_RUN/DEBIAN/postrm <<EOF
#!/bin/sh

set -e

if [ "\$1" = "remove" -o "\$1" = "purge" ]; then
    if [ ! -d /usr/lib/obtools ]; then
        grep -vs ^/usr/lib/obtools\$ /etc/ld.so.conf > /etc/ld.so.conf.new
        mv /etc/ld.so.conf.new /etc/ld.so.conf
    fi
    ldconfig
fi
EOF

chmod 0755 $DEB_RUN/DEBIAN/postinst $DEB_RUN/DEBIAN/postrm

# 'install' files
install -s -m 0644 $SRC_DIR/${PACKAGE}.so.${VERSION} $DEB_RUN/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-cppt.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-regen.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-xmi.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-cppt.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-regen.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-xmi.h $DEB_DEV/usr/include/obtools/

# create links
pushd $DEB_RUN/usr/lib/obtools
ln -s ${PACKAGE}.so.${VERSION} ${PACKAGE}.so.${VERSIONM}
cd $DEB_DEV/usr/lib/obtools
ln -s ${PACKAGE}.so.${VERSIONM} ${PACKAGE}.so
popd

fakeroot -- dpkg -b $DEB_RUN $SRC_DIR/${PACKAGE}_${VERSION}-${RELEASE}_${ARCH}.deb
fakeroot -- dpkg -b $DEB_DEV $SRC_DIR/${PACKAGE}-dev_${VERSION}-${RELEASE}_${ARCH}.deb

# clean up
rm -rf $DEB_RUN $DEB_DEV
