//==========================================================================
// ObTools::Gather: segment.cc
//
// Implementation of gather buffer segment
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gather.h"

namespace ObTools { namespace Gather {

Segment::Segment(const Segment& s):
  owned_data(0), data(0), length(0)
{
  *this = s;
}

Segment& Segment::operator=(const Segment& s)
{
  // Free any owned memory
  if (owned_data)
  {
    delete[] owned_data;
    owned_data = 0;
  }

  if (s.owned_data)
  {
    owned_data = new data_t[s.length];
    data = owned_data;
    length = s.length;
    memcpy(owned_data, s.owned_data, length);
  }
  else
  {
    data = s.data;
    length = s.length;
  }

  return *this;
}

}} // namespaces
