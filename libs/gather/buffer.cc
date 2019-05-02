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

BufferIterator::value_type BufferIterator::end(0);

//--------------------------------------------------------------------------
// Get total length of data in buffer
length_t Buffer::get_length() const
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

  // Copy over existing, shallow copy
  for(unsigned int i=0; i<count; i++) new_segments[i].take(segments[i]);

  // Reset new ones
  for(unsigned int i=count; i<new_size; i++) new_segments[i].reset();

  // Replace
  delete[] segments;
  segments = new_segments;
  size = new_size;
}

//--------------------------------------------------------------------------
// Add a segment at the end, extending if required
// Segment is 'taken' - seg.owned_data is copied by ref and cleared
// Returns the added segment
Segment& Buffer::add(Segment& seg)
{
  if (count >= size) resize(size ? size*2 : 1);
  return segments[count++].take(seg);
}

//--------------------------------------------------------------------------
// Add another buffer to the end of this one, copying any owned_data
void Buffer::add(const Buffer& buffer)
{
  // Copy, with memory copying of owned data
  for(unsigned int i=0; i<buffer.count; i++)
  {
    Segment seg;
    seg.copy(buffer.segments[i]);
    add(seg);
  }
}

//--------------------------------------------------------------------------
// Insert a segment at the given index (default 0, the beginning),
// extending if required
// Takes the data from seg and clears it
// Returns the added segment
Segment& Buffer::insert(Segment& seg, unsigned int pos)
{
  if (count >= size) resize(size ? size*2 : 1);
  if (pos > count) abort();

  // Shift up one
  for(unsigned int i=count++; i>pos; i--) segments[i].take(segments[i-1]);

  return segments[pos].take(seg);
}

//--------------------------------------------------------------------------
// Copy some data to a contiguous buffer
length_t Buffer::copy(data_t *data, length_t offset, length_t len) const
{
  length_t gather_offset = 0;
  length_t data_read = 0;

  for (unsigned i = 0; i < count; ++i)
  {
    const Segment &segment = segments[i];
    if (offset < gather_offset + segment.length)
    {
      length_t start = (gather_offset > offset ? 0 : offset - gather_offset);
      length_t to_read = len - data_read;
      if (to_read > segment.length - start)
        to_read = segment.length - start;
      memcpy(data, &segment.data[start], to_read);
      data += to_read;
      data_read += to_read;
      if (data_read >= len)
        break;
    }
    gather_offset += segment.length;
  }
  return data_read;
}

//--------------------------------------------------------------------------
// Copy some data to a contiguous buffer from an iterator position
length_t Buffer::copy(data_t *data, const BufferIterator& offset,
                      length_t len) const
{
  Segment *s = offset.get_segment();
  if (!s || s < segments || s >= &segments[count])
    return 0;

  length_t segment_offset = offset.get_segment_offset();
  length_t data_read = 0;

  for (; s < &segments[count]; ++s)
  {
    const Segment &segment = *s;
    length_t to_read = len - data_read;
    if (to_read > segment.length - segment_offset)
      to_read = segment.length - segment_offset;
    memcpy(data, &segment.data[segment_offset], to_read);
    data += to_read;
    data_read += to_read;
    if (data_read >= len)
      break;
    segment_offset = 0;
  }
  return data_read;
}

#if !defined(PLATFORM_WINDOWS)
//--------------------------------------------------------------------------
// Fill an iovec array with the data
// iovec must be pre-allocated to the maximum segments of the buffer (size)
// Returns the number of segments filled in, or size+1 if it overflowed
unsigned int Buffer::fill(struct iovec *iovec, unsigned int size)
{
  unsigned int n=0;
  for(unsigned int i=0; i<count; i++)
  {
    Segment &seg = segments[i];
    if (seg.length)
    {
      // Check we haven't overflowed
      if (n >= size) return size+1;

      iovec->iov_base = static_cast<void *>(seg.data);
      iovec->iov_len  = seg.length;
      iovec++; n++;
    }
  }

  return n;
}
#endif

//--------------------------------------------------------------------------
// Dump the buffer to the given stream, optionally with data as well
void Buffer::dump(ostream& sout, bool show_data) const
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
// Tidy up a buffer, shuffling occupied segments back over empty ones
void Buffer::tidy()
{
  queue<Segment *> free_segments;
  unsigned int new_count(0);

  for (Segment *segment = &segments[0]; segment < &segments[count];
       ++segment)
  {
    if (segment->length)
    {
      if (free_segments.size())
      {
        Segment *first_free = free_segments.front();
        first_free->take(*segment);
        free_segments.pop();
        segment->consume(segment->length);
        free_segments.push(segment);
      }
      new_count++;
    }
    else
    {
      free_segments.push(segment);
    }
  }
  count = new_count;
}

//--------------------------------------------------------------------------
// Add references to a run of data from another buffer
void Buffer::add(const Buffer& buffer, length_t offset, length_t len)
{
  length_t gather_offset = 0;
  length_t data_read = 0;

  const Segment *segments_b = buffer.get_segments();

  for (unsigned i = 0; i < buffer.get_count(); ++i)
  {
    const Segment &segment = segments_b[i];
    if (offset < gather_offset + segment.length)
    {
      length_t start = (gather_offset > offset ? 0 : offset - gather_offset);
      length_t to_read = len - data_read;
      if (to_read > segment.length - start)
        to_read = segment.length - start;
      add(&segment.data[start], to_read);
      data_read += to_read;
      if (data_read >= len)
        break;
    }
    gather_offset += segment.length;
  }
}

//--------------------------------------------------------------------------
// Get a flattened data pointer at the given offset and length, multiple
// segment version (see ot-gather.h for single-segment optimised inline)
data_t *Buffer::get_flat_data_multi(length_t offset, length_t length,
                                    data_t *temp_buf)
{
  // Find the starting segment
  for(unsigned int i=0; i<count; i++)
  {
    Segment &seg = segments[i];
    length_t slen = seg.length;

    // Starts in this one?
    if (offset < slen)
    {
      // All in this one?
      if (offset + length < slen)
        return seg.data+offset;
      else
      {
        // We need to flatten multiple segments into temp_buf...

        // First the remainder of this segment
        length_t copied = slen-offset;
        memcpy(temp_buf, seg.data+offset, copied);

        // Then following segments to make up the rest
        for(++i; copied<length; i++)
        {
          // If we run out of segments, fail
          if (i >= count) return 0;

          Segment &seg2 = segments[i];

          // Copy as much as we need, or whole segment if less
          length_t needed = length-copied;
          if (needed > seg2.length) needed = seg2.length;
          memcpy(temp_buf+copied, seg2.data, needed);
          copied += needed;
        }

        // We completed - return their buffer back
        return temp_buf;
      }
    }

    offset -= seg.length;
  }

  // Offset off the end of the buffer - fail
  return 0;
}

//--------------------------------------------------------------------------
// Replace data from a flat buffer at the given offset
// Data must already exist, otherwise it just stops at end of existing data
void Buffer::replace(length_t offset, const data_t *buf, length_t length)
{
  // Find the starting segment
  for(unsigned int i=0; i<count; i++)
  {
    Segment &seg = segments[i];
    length_t slen = seg.length;

    // Starts in this one?
    if (offset < slen)
    {
      // All in this one?  Fast case
      if (offset + length < slen)
      {
        memcpy(seg.data+offset, buf, length);
      }
      else
      {
        // First the remainder of this segment
        length_t copied = slen-offset;
        memcpy(seg.data+offset, buf, copied);

        // Then following segments to make up the rest
        for(++i; copied<length && i<count; i++)
        {
          Segment &seg2 = segments[i];

          // Copy as much as we need, or whole segment if less
          length_t needed = length-copied;
          if (needed > seg2.length) needed = seg2.length;
          memcpy(seg2.data, buf+copied, needed);
          copied += needed;
        }
      }

      // Done either way
      break;
    }

    offset -= seg.length;
  }
}

//--------------------------------------------------------------------------
// Destructor
Buffer::~Buffer()
{
  reset();
  delete[] segments;
}


}} // namespaces
