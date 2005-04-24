#!/bin/sh
#==========================================================================
# Makedeb file for ObTools General libraries.
#
# Copyright (c) 2004 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

set -e

NAME=ot-xmlmesh-server
DESC='xmlmesh server'
PACKAGE=$NAME
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

mkdir -p $DEB_RUN/DEBIAN
mkdir -p $DEB_RUN/usr/bin/obtools

# create control and postinst
cat > $DEB_RUN/DEBIAN/control <<EOF
Package: $PACKAGE
Version: ${VERSION}-${RELEASE}
Section: libs
Priority: optional
Architecture: $ARCH
Depends: libc6 (>= 2.3.2), libstdc++5 (>= 3.3.3), libot-general (>= 1.0.0), libot-xmlmesh (>= 1.0.0)
Maintainer: Will Colwyn <willc@westerntelecom.co.uk>
Description: ObTools XMLMesh server - $DESC.
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

if [ "\$1" = "remove" ]; then
    ldconfig
fi
EOF

chmod 0755 $DEB_RUN/DEBIAN/postinst $DEB_RUN/DEBIAN/postrm

# 'install' files
install -s -m 0644 $SRC_DIR/${PACKAGE}.so.${VERSION} $DEB_RUN/usr/lib/obtools/

fakeroot -- dpkg -b $DEB_RUN $SRC_DIR/${PACKAGE}_${VERSION}-${RELEASE}_${ARCH}.deb
fakeroot -- dpkg -b $DEB_DEV $SRC_DIR/${PACKAGE}-dev_${VERSION}-${RELEASE}_${ARCH}.deb

# clean up
rm -rf $DEB_RUN
