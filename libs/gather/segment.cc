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

//--------------------------------------------------------------------------
// Reset to new length
void Segment::reset(length_t n)
{
  length=n;
  if (!n)
  {
    if (owned_data) delete[] owned_data;
    owned_data = data = 0;
  }
}

//--------------------------------------------------------------------------
// Copy data from another segment - does a deep copy of the owned_data of
// the other segment, returns *this
Segment& Segment::copy(const Segment& s)
{
  // Free any owned memory
  if (owned_data) // GCOV_EXCL_START - copy() only called on fresh segments
  {
    delete[] owned_data;
    owned_data = 0;
  } // GCOV_EXCL_STOP

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
