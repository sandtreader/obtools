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
MAINEXE=""

for file in WINDOWS/files WINDOWS/external-files
do
  if [ -e $file ]
  then
    while read l
    do
      set -o noglob $l
      f=$(basename $1)
      if [ -z "$MAINEXE" ]
      then
        MAINEXE="$1"
      fi
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

UNINSTREG="Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\\${APPNAME}"
makensis -V4 - <<EOF
$PLUGINDIRS
!define APPNAME "$APPNAME"
!include "FileFunc.nsh"

Outfile $OUTFILE

InstallDir "\$PROGRAMFILES64\\\${APPNAME}"

Section "Install"
  SetOutPath \$INSTDIR
$INSTALL
  SetShellVarContext all
  CreateShortCut "\$SMPROGRAMS\\\${APPNAME}.lnk" "\$INSTDIR\\$MAINEXE"
  WriteUninstaller "\$INSTDIR\\uninstall.exe"
  WriteRegStr HKLM "$UNINSTREG" "DisplayName" "\${APPNAME}"
  WriteRegStr HKLM "$UNINSTREG" "UninstallString" "$\\"\$INSTDIR\\uninstall.exe$\\""
  WriteRegStr HKLM "$UNINSTREG" "QuietUninstallString" "$\\"\$INSTDIR\\uninstall.exe$\\" /S"
  WriteRegStr HKLM "$UNINSTREG" "InstallLocation" "\$INSTDIR"
  WriteRegStr HKLM "$UNINSTREG" "Publisher" "$PUBLISHER"
  WriteRegStr HKLM "$UNINSTREG" "DisplayVersion" "$VERSION"
  WriteRegStr HKLM "$UNINSTREG" "DisplayIcon" "\$INSTDIR\\$MAINEXE"
  \${GetSize} "\$INSTDIR" "/S=0K" \$0 \$1 \$2
  IntFmt \$0 "0x%08X" \$0
  WriteRegDWORD HKLM "$UNINSTREG" "EstimatedSize" "\$0"
SectionEnd

Section "Uninstall"
  SetShellVarContext all
  Delete "\$SMPROGRAMS\\${APPNAME}.lnk"
  DeleteRegKey HKLM "$UNINSTREG"
$UNINSTALL
  Delete "\$INSTDIR\\uninstall.exe"
$RMDIRS
  RmDir \$INSTDIR
SectionEnd
EOF
