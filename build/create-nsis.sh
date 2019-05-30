#!/bin/sh

set -e

VERSION=$1
REVISION=$2
OUTFILE=$3

. WINDOWS/control

INSTALL=""
UNINSTALL=""
DIRS=""
PLUGINDIRS=""

for file in WINDOWS/files WINDOWS/external-files
do
  if [ -e $file ]
  then
    while read l
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
        DIRS="$DIRS
$2"
        INSTALL="$INSTALL
  SetOutPath \"\$INSTDIR\\$2\"
  File $1"
        UNINSTALL="$UNINSTALL
  Delete \"\$INSTDIR\\$2\\$f\""
      fi
    done < $file
  fi
done

if [ -e WINDOWS/services ]
then
  PLUGINDIRS="$PLUGINDIRS
!AddPluginDir /home/shared/tools/windows/nsis/simple-service/"
  while read l
  do
    set $l
    INSTALL="$INSTALL
  SimpleSC::InstallService \"$APPNAME\" \"$APPNAME\" \"16\" \"2\" \"\$INSTDIR\\$1\" \"\" \"\" \"\""
    UNINSTALL="SimpleSC::RemoveService \"$APPNAME\"
$UNINSTALL"
  done < WINDOWS/services
fi

RMDIRS=""
for dir in `echo "$DIRS" | sort -ur -`
do
  RMDIRS="$RMDIRS
  RmDir \$INSTDIR\\$dir"
done

makensis - <<EOF
$PLUGINDIRS
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
$RMDIRS
  RmDir \$INSTDIR
SectionEnd
EOF
