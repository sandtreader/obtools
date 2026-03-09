#!/bin/sh

set -e

DISTRO=$(lsb_release -s -i)

VERSION=$1
REVISION=$2
NAME=$3
OUTPUT_FILE=$4
shift 4

# Parse args: -- DEBIAN_FILES -- PRODUCT_FILES
DEBIAN_FILES=""
PRODUCT_FILES=""
mode=start
for arg in "$@"; do
  if [ "$arg" = "--" ]; then
    case "$mode" in
      start) mode=debian ;;
      debian) mode=products ;;
    esac
    continue
  fi
  case "$mode" in
    debian) DEBIAN_FILES="$DEBIAN_FILES $arg" ;;
    products) PRODUCT_FILES="$PRODUCT_FILES $arg" ;;
  esac
done

# Output dir must be absolute because we're about to cd
SRCDIR=$PWD
OUTPUT_DIR=$PWD/$(dirname $OUTPUT_FILE)
echo Build package in $PWD to $OUTPUT_DIR

# Build in a temp dir (tup sandbox blocks cp -R and directory listing)
TMPDIR=$(mktemp -d)/build
mkdir -p $TMPDIR

# Copy files referenced by DEBIAN/* from source or build output
# (tup sandbox prevents cp -R and directory listing)
for debfile in $DEBIAN_FILES; do
  while IFS= read -r line; do
    case "$line" in \#!*|"") continue ;; esac
    src=$(echo "$line" | awk '{print $1}')
    [ -z "$src" ] && continue
    if [ -f "$src" ]; then
      mkdir -p "$TMPDIR/$(dirname "$src")"
      cp "$src" "$TMPDIR/$src"
    elif [ -f "$OUTPUT_DIR/$src" ]; then
      mkdir -p "$TMPDIR/$(dirname "$src")"
      cp "$OUTPUT_DIR/$src" "$TMPDIR/$src"
    fi
  done < "$debfile"
done

# Copy build products
for f in $PRODUCT_FILES; do
  if [ -f "$OUTPUT_DIR/$f" ]; then
    cp "$OUTPUT_DIR/$f" "$TMPDIR/"
  elif [ -f "$f" ]; then
    cp "$f" "$TMPDIR/"
  fi
done

cd $TMPDIR

DEBDIR=debian

CHANGELOG=$DEBDIR/changelog
COMPAT=$DEBDIR/compat
RULES=$DEBDIR/rules

# Copy DEBIAN packaging files
mkdir -p $DEBDIR
for f in $DEBIAN_FILES; do
  cp "$SRCDIR/$f" $DEBDIR/
done

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
10
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
