//==========================================================================
// ObTools::ReGen: regen.cc
//
// Command line regeneration utility
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-regen.h"
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <string.h>

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  int flags = 0;
  const char *marker = "//~";

  if (argc < 3)
  {
    cout << "Usage:\n";
    cout << "  " << argv[0] << " [options] <user file> <master file>\n\n";
    cout << "Options:\n";
    cout << "  --delete-orphans, -d   Delete orphan blocks left in user file\n";
    cout << "  --suppress-new, -s     Suppress new blocks from master file\n";
    cout << "  --marker, -m <marker>  Set marker comment [//~]\n";
    return 0;
  }

  // Get options, leave i as first non-option
  int i;
  for(i=1; i<argc-2; i++)
  {
    char *arg=argv[i];
    if (!strcmp(arg, "-d") || !strcmp(arg, "--delete-orphans"))
      flags |= ObTools::ReGen::MERGE_DELETE_ORPHANS;
    else if (!strcmp(arg, "-s") || !strcmp(arg, "--suppress-new"))
      flags |= ObTools::ReGen::MERGE_SUPPRESS_NEW;
    else if ((!strcmp(arg, "-m") || !strcmp(arg, "--marker")) && i<argc-1)
      marker = argv[++i];
    else
    {
      cerr << "Unknown option: " << arg;
      return 4;
    }
  }
  
  char *userfn = argv[i++];
  char *masterfn = argv[i++];

  ifstream masterfile(masterfn);

  if (!masterfile)
  {
    cerr << "Can't open master file " << masterfn << endl;
    return 2;
  }

  // Create regenerating output file
  ObTools::ReGen::rofstream outfile(userfn, marker, flags);

  // Spool master file directly to it
  while (masterfile)
  {
    char c;
    masterfile.get(c);
    outfile << c;
  }
  masterfile.close();

  // Close the output - this does all the work
  outfile.close();

  return 0;
}



