//==========================================================================
// ObTools::Gather: buffer.cc
//
// Implementation of gather buffer
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gather.h"

namespace ObTools { namespace Gather {

//--------------------------------------------------------------------------
// Extend the buffer by doubling its segment array size
// Guarantees to extend buffer by at least 1 entry
void Buffer::extend()
{
  // !!!
}

//--------------------------------------------------------------------------
// Add a segment at the end, extending if required
// Returns the added segment
Segment& Buffer::add_segment(const Segment& seg)
{
  if (count >= size) extend();
  return segments[count++] = seg;
}


//--------------------------------------------------------------------------
// Destructor
Buffer::~Buffer()
{
  for(int i=0; i<count; i++) segments[i].destroy();
  delete[] segments;
}


}} // namespaces
