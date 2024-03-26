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

 -- ObTools support <support@obtools.com>  `date -R`
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
  sed  -i 's/^Build-Depends: debhelper (>= 9)/#Build-Depends:/g' debian/control
  cat <<EOF >> $RULES
override_dh_shlibdeps:
override_dh_strip_nondeterminism:
EOF

  TOP_DIR=${PWD%%/.tup/*}
fi

if [ `id -u` -eq 0 ]; then
  FAKEROOT=
fi

DEB_BUILD_OPTIONS=noautodbgsym dpkg-buildpackage -uc -b $FAKEROOT -tc

for NAME in $NAMES
do
  PACKAGE=${NAME}_${VERSION}-${REVISION}_*.deb
  mv ../${PACKAGE} ./
  rm ../${NAME}_${VERSION}-${REVISION}_*.changes
  rm -f ../${NAME}_${VERSION}-${REVISION}_*.buildinfo

  if [ "$DISTRO" = "CentOS" ]; then
    . $TOP_DIR/create-rpm.sh
    rm $PACKAGE
  fi
done

rm -rf $DEBDIR
