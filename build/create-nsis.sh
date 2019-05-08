#!/bin/sh

set -e

VERSION=$1
REVISION=$2
OUTFILE=$3

. WINDOWS/control

INSTALL=""
UNINSTALL=""

for file in WINDOWS/files WINDOWS/external-files
do
  if [ -e $file ]
  then
    while read l;
    do
      set $l
      f=$(basename $1)
      if [ -z "$2" ]
      then
        INSTALL="$INSTALL
  SetOutPath \"\$INSTDIR\"
  File $1"
        UNINSTALL="$UNINSTALL
  Delete \"\$INSTDIR\\$f\""
      else
      INSTALL="$INSTALL
  SetOutPath \"\$INSTDIR\\$2\"
  File $1"
      UNINSTALL="$UNINSTALL
  Delete \"\$INSTDIR\\$2\\$f\""
      fi
    done < $file
  fi
done

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
