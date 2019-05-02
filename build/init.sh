#!/bin/bash

BUILD_DIR=`dirname $0`
ln -sf $BUILD_DIR/Tuprules.tup
ln -sf $BUILD_DIR/Tuprules.lua
ln -sf $BUILD_DIR/create-deb.sh
ln -sf $BUILD_DIR/create-rpm.sh

platform=linux

while getopts ":t:p:" opt; do
  case $opt in
    t)
      buildtype=$OPTARG
      ;;
    p)
      platform=$OPTARG
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

  if [ "${HOSTTYPE:0:3}" =  "arm" ]; then
    TARGARCH=armhf
  fi

  if [ "$HOSTTYPE" =  "x86_64" ]; then
    TARGARCH=amd64
  fi

  mkdir build-$buildtype

  TUPCONFIG=build-$buildtype/tup.config
  GENERIC=$BUILD_DIR/tup.config.$buildtype
  SPECIFIC=$GENERIC.$TARGARCH

  FOUNDCONFIG=

  # try most specific first
  if [ -f $SPECIFIC ]; then
    cp $SPECIFIC $TUPCONFIG
    FOUNDCONFIG=1
  else
    if [ -f $GENERIC ]; then
      cp $GENERIC $TUPCONFIG
      echo CONFIG_ARCH=$TARGARCH >> $TUPCONFIG
      FOUNDCONFIG=1
    fi
  fi

  if [ $FOUNDCONFIG ]; then
    echo CONFIG_PLATFORM=$platform >> $TUPCONFIG
    tup init
  else
    echo "`basename $0`: can't find config for $buildtype, architecture $TARGARCH"
  fi
fi

