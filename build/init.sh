#!/bin/sh

BUILD_DIR=`dirname $0`
ln -sf $BUILD_DIR/Tuprules.tup
ln -sf $BUILD_DIR/Tuprules.lua
ln -sf $BUILD_DIR/create-deb.sh

while getopts ":t:" opt; do
  case $opt in
    t)
      buildtype=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
   :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

if [ "$buildtype" ]; then
  HOSTTYPE=`uname -m`
  TARGARCH=i386

  if [ "$HOSTTYPE" =  "x86_64" ]; then
    TARGARCH=amd64
  fi

  for config in $buildtype.$TARGARCH $buildtype
  do
    if [ -f $BUILD_DIR/tup.config.$config ]; then
      mkdir build-$buildtype
      cp $BUILD_DIR/tup.config.$config build-$buildtype/tup.config
      FOUNDCONFIG=1
      break
    fi
  done
  if [ $FOUNDCONFIG ]; then
    tup init
  else
    echo "`basename $0`: can't find config for $buildtype"
  fi
fi

