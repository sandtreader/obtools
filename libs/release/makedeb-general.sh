#!/bin/sh
#==========================================================================
# Makedeb file for ObTools General libraries.
#
# Copyright (c) 2004 xMill Consulting Limited.  All rights reserved
# @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
#==========================================================================

set -e

NAME=ot-general
DESC='general libraries'
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
Maintainer: Will Colwyn <willc@westerntelecom.co.uk>
Description: ObTools shared libraries - $DESC.
EOF

cat > $DEB_DEV/DEBIAN/control <<EOF
Package: ${PACKAGE}-dev
Version: ${VERSION}-${RELEASE}
Section: libs
Priority: optional
Architecture: $ARCH
Depends: $PACKAGE (>= $VERSION-$RELEASE)
Maintainer: Will Colwyn <willc@westerntelecom.co.uk>
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
install -s -m 0644 $SRC_DIR/${PACKAGE}-single.so.${VERSION} $DEB_RUN/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-chan.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-cli.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-init.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-log.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-misc.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-mt.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-net.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-text.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-time.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-web.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-xml.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-chan-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-init-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-log-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-misc-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-net-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-text-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-time-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-web-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-xml-single.a $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-cache.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-chan.h $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-cli-telnet.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-cli.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-init.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-log.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-misc.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-mt.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-net.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-text.h $DEB_DEV/usr/include/obtools/
install -m 0644 $SRC_DIR/ot-time.h $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-web.h $DEB_DEV/usr/lib/obtools/
install -m 0644 $SRC_DIR/ot-xml.h $DEB_DEV/usr/include/obtools/

# create links
pushd $DEB_RUN/usr/lib/obtools
ln -s ${PACKAGE}.so.${VERSION} ${PACKAGE}.so.${VERSIONM}
ln -s ${PACKAGE}-single.so.${VERSION} ${PACKAGE}-single.so.${VERSIONM}
cd $DEB_DEV/usr/lib/obtools
ln -s ${PACKAGE}.so.${VERSIONM} ${PACKAGE}.so
ln -s ${PACKAGE}-single.so.${VERSIONM} ${PACKAGE}-single.so
popd

fakeroot -- dpkg -b $DEB_RUN $SRC_DIR/${PACKAGE}_${VERSION}-${RELEASE}_${ARCH}.deb
fakeroot -- dpkg -b $DEB_DEV $SRC_DIR/${PACKAGE}-dev_${VERSION}-${RELEASE}_${ARCH}.deb

# clean up
rm -rf $DEB_RUN $DEB_DEV
