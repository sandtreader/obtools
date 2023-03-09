#!/bin/bash

set -e

VERSION=$1
REVISION=$2
OUTFILE=$3

PRELOADS=""

for file in WEB/files
do
  if [ -e $file ]
  then
    while read -r l
    do
      set -o noglob $l
      FROM=$1
      TO="$2/`basename $1`"
      PRELOADS="$PRELOADS $FROM@$TO"
    done < $file
  fi
done

~/emsdk/upstream/emscripten/tools/file_packager.py "$OUTFILE-$VERSION-$REVISION.data" --preload $PRELOADS --js-output="$OUTFILE-$VERSION-$REVISION.js"

cat "$OUTFILE.js" >> "$OUTFILE-$VERSION-$REVISION.js"

