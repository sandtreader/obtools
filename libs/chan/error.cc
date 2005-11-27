//==========================================================================
// ObTools::Chan: error.cc
//
// Channel error handling
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"

namespace ObTools { namespace Channel {

//------------------------------------------------------------------------
// << operator to write Error to ostream
// e.g. cout << e;
ostream& operator<<(ostream& s, const Error& e)
{
  s << "Channel error (" << e.error << "): " << e.text;
  return s;
}

}} // namespaces



