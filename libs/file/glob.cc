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

//--------------------------------------------------------------------------
// Constructor
Glob::Glob(const string& pattern)
{
#ifndef __WIN32__
  if (glob(pattern.c_str(), 0, 0, &result))
    throw(Error(strerror(errno)));
#endif
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
#ifndef __WIN32__
  return &result.gl_pathv[0];
#endif
}

//--------------------------------------------------------------------------
// Ending iterator
Glob::const_iterator Glob::end() const
{
#ifndef __WIN32__
  return &result.gl_pathv[result.gl_pathc];
#endif
}

//--------------------------------------------------------------------------
// Destructor
Glob::~Glob()
{
#ifndef __WIN32__
  globfree(&result);
#endif
}

}} // namespaces
