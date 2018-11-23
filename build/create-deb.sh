#!/bin/sh

set -e

DISTRO=$(lsb_release -s -i)

VERSION=$1
REVISION=$2
NAMES=""
NAME=""
while [ "$#" -gt 2 ]
do
  if [ "$NAME" = "" ]
  then
    NAME=$3
  fi
  NAMES=$NAMES" "$3
  shift
done

DEBDIR=debian

CHANGELOG=$DEBDIR/changelog
COMPAT=$DEBDIR/compat
RULES=$DEBDIR/rules
REMOVE=

cp -r DEBIAN $DEBDIR

if [ ! -e $CHANGELOG ]
then
  cat << EOF > $DEBDIR/changelog
$NAME ($VERSION-$REVISION) stable; urgency=low

  * See documentation.

 -- PacketShip Support <support@packetship.com>  `date -R`
EOF
fi

if [ ! -e $COMPAT ]
then
  cat << EOF > $COMPAT
9
EOF
fi

if [ ! -e $RULES ]
then
  cat << 'EOF' > $RULES
#!/usr/bin/make -f

%:
	dh $@
EOF
  chmod a+x $DEBDIR/rules
fi

FAKEROOT=-rfakeroot-ng

[ -x /usr/bin/fakeroot ] && FAKEROOT=-r/usr/bin/fakeroot
[ -x /usr/local/bin/fakeroot ] && FAKEROOT=-r/usr/bin/local/fakeroot

if [ -f /usr/local/bin/pseudo ]; then
  export PSEUDO_PREFIX=/usr/local
  FAKEROOT=-rpseudo
fi


if [ "$DISTRO" = "CentOS" ]; then
  FAKEROOT=-rfakeroot
  sed  -i 's/^Build-Depends: debhelper (>= 9)$/#Build-Depends:/g' debian/control
  cat <<EOF >> $RULES
override_dh_shlibdeps:
EOF

  # find build (scripts) directory
  RPM_DIR_DEPTH=0
  RPM_MAX_DEPTH=5
  RPM_MID=..
  while [ ! -d ${RPM_MID}/build -a $RPM_DIR_DEPTH -lt $RPM_MAX_DEPTH ]
  do
    RPM_MID=${RPM_MID}/..
    RPM_DIR_DEPTH=$(( ${RPM_DIR_DEPTH} + 1 ))
  done

  BUILD_DIR=${RPM_MID}/build
fi

if [ `id -u` -eq 0 ]; then
  FAKEROOT=
fi

dpkg-buildpackage -uc -b $FAKEROOT -tc

for NAME in $NAMES
do
  PACKAGE=${NAME}_${VERSION}-${REVISION}_*.deb
  mv ../${PACKAGE} ./
  rm ../${NAME}_${VERSION}-${REVISION}_*.changes
  rm -f ../${NAME}_${VERSION}-${REVISION}_*.buildinfo

  if [ "$DISTRO" = "CentOS" ]; then
    . $BUILD_DIR/create-rpm.sh
    rm $PACKAGE
  fi
done

rm -rf $DEBDIR
