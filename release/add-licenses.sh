#!/bin/sh
#Script to add licences to all files in a directory
#Parameters: as add-licence.pl

find . -type f -exec ~/world/obtools/release/add-licence-file.sh {} $1 \;

