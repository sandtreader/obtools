//==========================================================================
// ObTools::ReGen: regen.cc
//
// Command line regeneration utility
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-regen.h"
#include <fstream>
#include <cstdio>
#include <cerrno>

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

  ifstream userfile(userfn);
  ifstream masterfile(masterfn);

  if (!masterfile)
  {
    cerr << "Can't open master file " << masterfn << endl;
    return 2;
  }

  // Build new filename and output stream
  string newfn(userfn);
  newfn+="##";
  ofstream outfile(newfn.c_str());
  if (!outfile)
  {
    cerr << "Can't create temporary file " << newfn << endl;
    return 2;
  }

  // If userfile not readable, just spool master straight out
  if (!userfile)
  {
    while (masterfile)
    {
      char c;
      masterfile.get(c);
      outfile << c;
    }
  }
  else
  {
    // Merge with existing
    ObTools::ReGen::MarkedFile ufile(userfile);
    ObTools::ReGen::MasterFile mfile(masterfile, marker);

    mfile.merge(ufile, outfile, flags);
  }

  // Close output and rename to existing file
  userfile.close();
  outfile.close();
  masterfile.close();

  if (rename(newfn.c_str(), userfn))
  {
    cerr << "Rename of " << newfn << " to " << userfn << " failed - " << strerror(errno) << endl;
    return 2;
  }

  return 0;
}



