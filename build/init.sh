#!/bin/sh

BUILD_DIR=`dirname $0`
ln -s $BUILD_DIR/Tuprules.tup
ln -s $BUILD_DIR/Tupdepends.lua
ln -s $BUILD_DIR/Tupsources.lua
ln -s $BUILD_DIR/create-deb.sh
