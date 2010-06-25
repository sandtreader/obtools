//==========================================================================
// ObTools::Gather: buffer.cc
//
// Implementation of gather buffer
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gather.h"
#include "ot-misc.h"
#include "stdlib.h"

namespace ObTools { namespace Gather {

//--------------------------------------------------------------------------
// Extend the buffer by doubling its segment array size
// Guarantees to extend buffer by at least 1 entry
void Buffer::extend()
{
  // Create new segment array with double the size
  Segment *new_segments = new Segment[size*2];

  // Copy over existing
  for(unsigned int i=0; i<count; i++) new_segments[i]=segments[i];

  // Replace
  delete[] segments;
  segments = new_segments;
  size *= 2;
}

//--------------------------------------------------------------------------
// Add a segment at the end, extending if required
// Returns the added segment
Segment& Buffer::add(const Segment& seg)
{
  if (count >= size) extend();
  return segments[count++] = seg;
}

//--------------------------------------------------------------------------
// Insert a segment at the given index (default 0, the beginning), 
// extending if required
// Returns the added segment
Segment& Buffer::insert(const Segment& seg, unsigned int pos)
{
  if (count >= size) extend();
  if (pos >= count) abort();

  // Shift up one
  for(unsigned int i=count++; i>pos; i--) segments[i]=segments[i-1];
  return segments[pos] = seg;
}

//--------------------------------------------------------------------------
// Dump the buffer to the given stream, optionally with data as well
void Buffer::dump(ostream& sout, bool show_data)
{
  Misc::Dumper dumper(sout);

  sout << "Buffer (" << count << "/" << size << "):\n";
  length_t total = 0;
  for(unsigned int i=0; i<count; i++)
  {
    Segment &seg = segments[i];
    sout << (seg.owned?"* ":"  ") << seg.length << endl;
    if (show_data) dumper.dump(seg.data, seg.length);
    total += seg.length;
  }
  sout << "Total length " << total << endl;
}

//--------------------------------------------------------------------------
// Destructor
Buffer::~Buffer()
{
  for(unsigned int i=0; i<count; i++) segments[i].destroy();
  delete[] segments;
}


}} // namespaces
