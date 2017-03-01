#!/bin/sh

set -e

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
if [ -f /usr/local/bin/pseudo ]; then
  export PSEUDO_PREFIX=/usr/local
  FAKEROOT=-rpseudo
fi

if [ `id -u` -eq 0 ]; then
  FAKEROOT=
fi

dpkg-buildpackage -uc -b $FAKEROOT -tc
for NAME in $NAMES
do
  mv ../${NAME}_${VERSION}-${REVISION}_*.deb ./
  rm ../${NAME}_${VERSION}-${REVISION}_*.changes
  rm -f ../${NAME}_${VERSION}-${REVISION}_*.buildinfo
done
rm -rf $DEBDIR
