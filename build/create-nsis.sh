#!/bin/sh

set -e

VERSION=$1
REVISION=$2
OUTFILE=$3

. WINDOWS/control

INSTALL=""
UNINSTALL=""

if [ -e WINDOWS/files ]
then
  while read l;
  do
    INSTALL="$INSTALL
  File $l"
    f=$(basename $l)
    UNINSTALL="$UNINSTALL
  Delete \"\$INSTDIR\\$f\""
  done < WINDOWS/files
fi

if [ -e WINDOWS/external-files ]
then
  while read l;
  do
    INSTALL="$INSTALL
  File $l"
    f=$(basename $l)
    UNINSTALL="$UNINSTALL
  Delete \"\$INSTDIR\\$f\""
  done < WINDOWS/external-files
fi

makensis - <<EOF
!define APPNAME "$APPNAME"

Outfile $OUTFILE

InstallDir "\$PROGRAMFILES64\\\${APPNAME}"

Section "Install"
  SetOutPath \$INSTDIR
$INSTALL
  WriteUninstaller "\$INSTDIR\\uninstall.exe"
SectionEnd

Section "Uninstall"
$UNINSTALL
  Delete "\$INSTDIR\\uninstall.exe"
  RmDir \$INSTDIR
SectionEnd
EOF
