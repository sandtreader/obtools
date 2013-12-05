//==========================================================================
// ObTools::File: glob.cc
//
// Glob implementation
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <string.h>
#include <errno.h>

namespace ObTools { namespace File {

#ifndef __WIN32__

//--------------------------------------------------------------------------
// Constructor
Glob::Glob(const string& pattern)
{
  if (glob(pattern.c_str(), 0, 0, &result))
    throw(Error(strerror(errno)));
}

//--------------------------------------------------------------------------
// Erase files / directories
bool Glob::erase() const
{
  for (const_iterator it = begin(); it != end(); ++it)
  {
    Path path(*it);
    if (!path.erase())
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------
// Beginning iterator
Glob::const_iterator Glob::begin() const
{
  return &result.gl_pathv[0];
}

//--------------------------------------------------------------------------
// Ending iterator
Glob::const_iterator Glob::end() const
{
  return &result.gl_pathv[result.gl_pathc];
}

//--------------------------------------------------------------------------
// Destructor
Glob::~Glob()
{
  globfree(&result);
}

#endif

}} // namespaces
