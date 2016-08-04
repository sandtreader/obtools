//==========================================================================
// ObTools::Chan: error.cc
//
// Channel error handling
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"

namespace ObTools { namespace Channel {

//--------------------------------------------------------------------------
// << operator to write Error to ostream
// e.g. cout << e;
ostream& operator<<(ostream& s, const Error& e)
{
  s << "Channel error (" << e.error << "): " << e.text;
  return s;
}

}} // namespaces



