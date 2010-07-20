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
// Get total length of data in buffer
length_t Buffer::get_length()
{
  length_t total = 0;
  for(unsigned int i=0; i<count; i++) total += segments[i].length;
  return total;
}

//--------------------------------------------------------------------------
// Resize the buffer to the given number of segments
void Buffer::resize(unsigned int new_size)
{
  if (new_size < count) abort();

  // Create new segment array with new size
  Segment *new_segments = new Segment[new_size];

  // Copy over existing
  for(unsigned int i=0; i<count; i++) new_segments[i]=segments[i];

  // Replace
  delete[] segments;
  segments = new_segments;
  size = new_size;
}

//--------------------------------------------------------------------------
// Add a segment at the end, extending if required
// Returns the added segment
Segment& Buffer::add(const Segment& seg)
{
  if (count >= size) resize(size*2);
  return segments[count++] = seg;
}

//--------------------------------------------------------------------------
// Insert a segment at the given index (default 0, the beginning), 
// extending if required
// Returns the added segment
Segment& Buffer::insert(const Segment& seg, unsigned int pos)
{
  if (count >= size) resize(size*2);
  if (pos >= count) abort();

  // Shift up one
  for(unsigned int i=count++; i>pos; i--) segments[i]=segments[i-1];
  return segments[pos] = seg;
}

//--------------------------------------------------------------------------
// Fill an iovec array with the data
// iovec must be pre-allocated to the maximum segments of the buffer
// Returns the number of segments filled in
unsigned int Buffer::fill(struct iovec *iovec)
{
  for(unsigned int i=0; i<count; i++, iovec++)
  {
    Segment &seg = segments[i];
    iovec->iov_base = (void *)seg.data;
    iovec->iov_len  = seg.length;
  }

  return count;
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
// Reset the buffer to be empty
void Buffer::reset() 
{
  for(unsigned int i=0; i<count; i++) segments[i].destroy();
  count = 0; 
}

//--------------------------------------------------------------------------
// Destructor
Buffer::~Buffer()
{
  reset();
  delete[] segments;
}


}} // namespaces