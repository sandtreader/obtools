#!/bin/sh

set -e

DISTRO=$(lsb_release -s -i)

VERSION=$1
REVISION=$2
NAME=$3
OUTPUT_FILE=$4

# Output dir must be absolute because we're about to cd
OUTPUT_DIR=$PWD/$(dirname $OUTPUT_FILE)
echo Build package in $PWD to $OUTPUT_DIR

# Copy both the source and current output to a temp dir and shift to that
TMPDIR=$(mktemp -d)/build
mkdir -p $TMPDIR
cp -R * $TMPDIR
cp -R $OUTPUT_DIR/* $TMPDIR
cd $TMPDIR

DEBDIR=debian

CHANGELOG=$DEBDIR/changelog
COMPAT=$DEBDIR/compat
RULES=$DEBDIR/rules

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

if [ `id -u` -eq 0 ]; then
  FAKEROOT=
fi

DEB_BUILD_OPTIONS=noautodbgsym dpkg-buildpackage -uc -b $FAKEROOT -tc

PACKAGE=${NAME}_${VERSION}-${REVISION}_*.deb
mv $TMPDIR/../$PACKAGE $OUTPUT_DIR/

rm -rf $TMPDIR
