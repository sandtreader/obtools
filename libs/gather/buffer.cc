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
  if (pos > count) abort();

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
  unsigned int n=0;
  for(unsigned int i=0; i<count; i++)
  {
    Segment &seg = segments[i];
    if (seg.length)
    {
      iovec->iov_base = (void *)seg.data;
      iovec->iov_len  = seg.length;
      iovec++; n++;
    }
  }

  return n;
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
    sout << (seg.owned_data?"* ":"  ") << seg.length << endl;
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
// Limit the buffer to hold at most a particular length of data
// Returns the actual length remaining (which may be less than limit)
length_t Buffer::limit(length_t length)
{
  length_t total = 0;
  unsigned int i;

  // Skip over segments that are needed
  for(i=0; i<count; i++)
  {
    total += segments[i].length;
    if (total >= length) // This one passes it?
    {
      segments[i].length -= total-length;  // Reduce this one
      total = length;
      break;  // Stop here
    }
  }

  // Destroy any remaining segments
  for(unsigned int j=++i; j<count; j++) segments[j].destroy();
  if (i<count) count = i;

  return total;
}

//--------------------------------------------------------------------------
// Consume N bytes of data from the front of the buffer
void Buffer::consume(length_t n)
{
  // Skip over segments that are needed
  for(unsigned int i=0; i<count; i++)
  {
    Segment& seg = segments[i];
    if (!seg.length) continue;  // Fast skip for empty ones

    if (seg.length > n)
    {
      // Just consume off this one, and stop
      seg.consume(n);
      break;
    }
    else
    {
      // Clear this one and continue to next
      n -= seg.length;
      seg.reset();
    }
  }
}

//--------------------------------------------------------------------------
// Add another buffer to the end of this one
void Buffer::add(const Buffer& buffer)
{
  for(unsigned int i=0; i<buffer.count; i++)
    add(buffer.segments[i]);
}

//--------------------------------------------------------------------------
// Destructor
Buffer::~Buffer()
{
  reset();
  delete[] segments;
}


}} // namespaces
