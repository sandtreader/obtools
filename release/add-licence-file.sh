#!/bin/sh
#Script to add licences to all files in a directory
#Parameters: as add-licence.pl

~/world/obtools/release/add-licence.pl $2 < $1 > $1.new && mv $1.new $1

