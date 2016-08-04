//==========================================================================
// ObTools::ReGen: rofstream.cc
//
// Regenerating ofstream - use like ofstream, magically does regeneration
// over existing files
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-regen.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <string.h>

using namespace ObTools::ReGen;

//==========================================================================
//regenbuf - a streambuf which just captures everything into a string, then
//uses the rest of the ReGen package to modify the file on close

//--------------------------------------------------------------------------
// regenbuf Constructor
regenbuf::regenbuf(string _fn, const char *_marker, int _flags):
  streambuf(), fn(_fn), closed(false), marker(_marker), flags(_flags)
{
  // use unbuffered IO, since we're going to buffer it anyway
  setp(0,0);
  setg(0,0,0);
}

//--------------------------------------------------------------------------
// regenbuf Destructor - force close
regenbuf::~regenbuf()
{
  close();
}

//--------------------------------------------------------------------------
// regenbuf close() function
// Does all the real work here!
void regenbuf::close()
{
  // Avoid doing this more than once
  if (closed) return;
  closed = true;

  ifstream userfile(fn.c_str());

  // Build new filename and output stream
  string newfn(fn);
  newfn+="##";
  ofstream outfile(newfn.c_str());
  if (!outfile)
  {
    cerr << "Can't create temporary file " << newfn << endl;
    return;
  }

  // If userfile not readable, just spool buffer straight out
  if (!userfile)
  {
    outfile << buffer;
  }
  else
  {
    // Merge with existing
    istringstream bufstr(buffer);
    ObTools::ReGen::MarkedFile ufile(userfile);
    ObTools::ReGen::MasterFile mfile(bufstr, marker);

    mfile.merge(ufile, outfile, flags);
  }

  // Close output and rename to existing file
  userfile.close();
  outfile.close();

  if (rename(newfn.c_str(), fn.c_str()))
  {
    cerr << "Rename of " << newfn << " to " << fn << " failed - "
         << strerror(errno) << endl;
  }
}

//--------------------------------------------------------------------------
// regenbuf overflow() function
// Handles characters one at a time, since streambuf is unbuffered
int regenbuf::overflow(int c)
{
  if (c==EOF)
    close();
  else
    buffer += static_cast<char>(c);

  return 0;
}


