//==========================================================================
// ObTools::Gather: ot-gather.h
//
// Public definitions for ObTools::Gather
//
// Multi-element gather buffer
//
// Allows complex creation of network packets etc. without memory copying
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_GATHER_H
#define __OBTOOLS_GATHER_H

#include <stdint.h>
#include <stdlib.h>
#include <ostream>
#include "ot-chan.h"

#if !defined(__WIN32__)
#include "sys/uio.h"
#endif

namespace ObTools { namespace Gather {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Types
typedef unsigned char data_t;
typedef unsigned long length_t;

//==========================================================================
// Individual segment of a buffer
class Segment
{
 private:
  // Block copy and assignment to avoid implicit copying horrors
  // - use 'take' or 'copy' explicitly
  Segment(const Segment&) {}
  void operator=(const Segment&) {}

 public:
  data_t *owned_data;      // Original allocated block, if we allocated it
  data_t *data;            // Unconsumed data start
  length_t length;         // Unconsumed data length

  //------------------------------------------------------------------------
  // Constructors
  // Default
  Segment(): owned_data(0), data(0), length(0) {}

  // Reference to external data
  Segment(data_t *_data, length_t _length):
    owned_data(0), data(_data), length(_length) {}

  // Reference to data allocated here
  Segment(length_t _length):
    owned_data(new data_t[_length]), data(owned_data), length(_length) {}

  //------------------------------------------------------------------------
  // Consume N bytes from the beginning
  void consume(length_t n) { data+=n; length-=n; }

  //------------------------------------------------------------------------
  // Reset to new length
  void reset(length_t n=0);

  //------------------------------------------------------------------------
  // Destroy - note, explicit call, not destructor
  void destroy() { if (owned_data) delete[] owned_data; owned_data=0; }

  //------------------------------------------------------------------------
  // Take the data from another segment, clearing any owned_data in the
  // original
  Segment& take(Segment& s)
  { owned_data = s.owned_data; s.owned_data = 0;
    data = s.data; length = s.length; return *this; }

  //------------------------------------------------------------------------
  // Copy data from another segment - does a deep copy of the owned_data of
  // the other segment, returns *this
  Segment& copy(const Segment& s);

  //------------------------------------------------------------------------
  // Destructor - check no owned data left
#if defined(DEBUG)
  ~Segment()
  {
    if (owned_data)
    {
      cerr << "Owned data left in Segment\n";
      abort();
    }
  }
#endif

};

//==========================================================================
// Gather buffer iterator
class BufferIterator
{
public:
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;
  typedef data_t value_type;
  typedef data_t& reference;
  typedef data_t *pointer;

private:
  Segment *segments;
  unsigned int count;
  Segment *segment;
  length_t countdown;
  pointer dp;

  inline bool is_end() const
  {
    return dp == &end;
  }

  void next_segment()
  {
    for (;;)
    {
      ++segment;
      if (segment >= &segments[count])
      {
        countdown = 0;
        dp = &end;
        return;
      }
      else if (segment->length)
      {
        countdown = segment->length;
        dp = &segment->data[0];
        return;
      }
    }
  }

  void previous_segment()
  {
    if (segment == segments)
    {
      countdown = segment->length;
      dp = &segment->data[0];
      return;
    }
    while (segment > segments && !(--segment)->length)
      ;
    if (segment->length)
    {
      countdown = 1;
      dp = &segment->data[segment->length - 1];
    }
    else
      next_segment();
  }

  static value_type end;

public:
  BufferIterator() {}
  BufferIterator(Segment *_segments, unsigned int _count,
                 Segment *_segment, length_t _countdown,
                 pointer _dp):
    segments(_segments), count(_count), segment(_segment),
    countdown(_countdown), dp(_dp)
  {
    if (segment >= &segments[count])
      dp = &end;
  }
  BufferIterator(const BufferIterator& bi):
    segments(bi.segments), count(bi.count), segment(bi.segment),
    countdown(bi.countdown), dp(bi.dp)
  {}

  Segment *get_segment() const
  {
    if (is_end())
      return 0;
    else
      return segment;
  }

  length_t get_segment_offset() const
  {
    if (is_end())
      return 0;
    else
      return segment->length - countdown;
  }

  inline bool operator==(const BufferIterator& bi) const
  {
    return dp == bi.dp;
  }

  inline bool operator!=(const BufferIterator& bi) const
  {
    return !operator==(bi);
  }

  BufferIterator& operator++()
  {
    if (is_end())
      return *this;
    if (!--countdown)
      next_segment();
    else
      ++dp;
    return *this;
  }

  BufferIterator& operator+=(length_t add)
  {
    while (add >= countdown)
    {
      if (is_end())
        return *this;
      add -= countdown;
      next_segment();
    }
    countdown -= add;
    dp += add;
    return *this;
  }

  BufferIterator operator+(length_t add)
  {
    BufferIterator added = *this;
    added += add;
    return added;
  }

  BufferIterator& operator--()
  {
    if (is_end() || ++countdown > segment->length)
      previous_segment();
    else
      --dp;
    return *this;
  }

  BufferIterator& operator-=(length_t sub)
  {
    while (is_end() || sub > segment->length - countdown)
    {
      if (segment == segments)
        throw("Attempt to decrement past start of buffer");
      if (is_end())
        --sub;
      else
        sub -= segment->length - countdown + 1;
      previous_segment();
    }
    countdown += sub;
    dp -= sub;
    return *this;
  }

  reference operator*()
  {
    return *dp;
  }

};

//==========================================================================
// Gather buffer
class Buffer
{
private:
  unsigned int size;      // Size allocated
  unsigned int count;     // Number of segments used
  Segment *segments;      // Allocated array of segments

  // Internal
  data_t *get_flat_data_multi(length_t offset, length_t length,
                              data_t *temp_buf);

public:
  typedef BufferIterator iterator;

  static const int DEFAULT_SIZE = 4;

  //------------------------------------------------------------------------
  // Constructor
  Buffer(unsigned int _size = DEFAULT_SIZE):
    size(_size), count(0), segments(new Segment[_size]) {}

  //------------------------------------------------------------------------
  // Get count of used segments
  unsigned int get_count() const { return count; }

  //------------------------------------------------------------------------
  // Get total number of segments allocated
  unsigned int get_size() const { return size; }

  //------------------------------------------------------------------------
  // Get segment list
  const Segment *get_segments() const { return segments; }

  //------------------------------------------------------------------------
  // Get total length of data in buffer
  length_t get_length() const;

  //------------------------------------------------------------------------
  // Resize the buffer to the given number of segments
  void resize(unsigned int new_size);

  //------------------------------------------------------------------------
  // Reset the buffer to be empty
  void reset();

  //------------------------------------------------------------------------
  // Limit the buffer to hold at most a particular length of data
  length_t limit(length_t length);

  //------------------------------------------------------------------------
  // Add a segment at the end, extending if required
  // Segment is shallow copied (owned_data not copied)
  // Returns the added segment
  Segment& add(Segment& seg);

  //------------------------------------------------------------------------
  // Add a segment from external data to the end of the buffer
  // Returns the added segment
  Segment& add(data_t *data, length_t length)
  { Segment seg(data, length); return add(seg); }

  //------------------------------------------------------------------------
  // Add a segment with allocated data to the end of the buffer
  // Returns the added segment
  Segment& add(length_t length)
  { Segment seg(length); return add(seg); }

  //------------------------------------------------------------------------
  // Add another buffer to the end of this one, copying any owned_data
  void add(const Buffer& buffer);

  //------------------------------------------------------------------------
  // Add references to a run of data from another buffer
  void add(const Buffer& buffer, length_t offset, length_t len);

  //------------------------------------------------------------------------
  // Insert a segment at the given index (default 0, the beginning),
  // extending if required
  // Takes the data from seg and clears it
  // Returns the added segment
  Segment& insert(Segment& seg, unsigned int pos);

  //------------------------------------------------------------------------
  // Insert a segment from external data at a given position (default 0, start)
  // Returns the inserted segment
  Segment& insert(data_t *data, length_t length, unsigned int pos=0)
  { Segment seg(data, length); return insert(seg, pos); }

  //------------------------------------------------------------------------
  // Insert a segment with allocated data at a given position (default 0,start)
  // Returns the inserted segment
  Segment& insert(length_t length, unsigned int pos=0)
  { Segment seg(length); return insert(seg, pos); }

  //------------------------------------------------------------------------
  // Consume N bytes of data from the front of the buffer
  void consume(length_t n);

  //------------------------------------------------------------------------
  // Tidy up a buffer, shuffling occupied segments back over empty ones
  void tidy();

  //------------------------------------------------------------------------
  // Copy some data to a contiguous buffer
  length_t copy(data_t *data, length_t offset, length_t len) const;

  //------------------------------------------------------------------------
  // Copy some data to a contiguous buffer from an iterator position
  length_t copy(data_t *data, const BufferIterator& offset,
                length_t len) const;

#if !defined(__WIN32__)
  //------------------------------------------------------------------------
  // Fill an iovec array with the data
  // iovec must be pre-allocated to the maximum segments of the buffer (size)
  // Returns the number of segments filled in, or size+1 if it overflowed
  unsigned int fill(struct iovec *iovec, unsigned int size);
#endif

  //------------------------------------------------------------------------
  // Get a flattened data pointer at the given offset and length, either
  // returning direct pointer into buffer's data segments if it doesn't
  // cross a boundary, or copying into the given temporary buffer if it does.
  // Returns pointer to flat data block or 0 if not enough data in buffer
  // to fulfill it
  // temp_buf must be allocated to at least 'length' bytes
  data_t *get_flat_data(length_t offset, length_t length, data_t *temp_buf)
  {
    // Fast inline optimisation for single segment
    if (count == 1)
    {
      Segment &seg = *segments;
      if (offset + length > seg.length) return 0;
      return seg.data + offset;
    }

    // Otherwise full multi-segment version
    return get_flat_data_multi(offset, length, temp_buf);
  }

  //------------------------------------------------------------------------
  // Replace data from a flat buffer at the given offset
  // Data must already exist, otherwise it just stops at end of existing data
  void replace(length_t offset, const data_t *buf, length_t length);

  //------------------------------------------------------------------------
  // Dump the buffer to the given stream, optionally with data as well
  void dump(ostream& sout, bool show_data=false) const;

  //------------------------------------------------------------------------
  // Iterator functions
  iterator begin() const
  {
    for (Segment *segment = segments; segment < &segments[count]; ++segment)
      if (segment->length)
        return BufferIterator(segments, count, segment,
                              segment->length, &segment->data[0]);
    return end();
  }

  iterator end() const
  {
    return BufferIterator(segments, count, &segments[count],
                          0, 0);
  }

  //------------------------------------------------------------------------
  // Destructor
  ~Buffer();
};

//==========================================================================
// Gather reader
class Reader: public Channel::Reader
{
private:
  const Buffer& buffer;
  BufferIterator it;

public:
  //------------------------------------------------------------------------
  // Constructor
  Reader(const Buffer& _buffer):
    buffer(_buffer), it(_buffer.begin())
  {}

  //------------------------------------------------------------------------
  // Read implementations - see ot-chan.h for details
  virtual size_t basic_read(void *buf, size_t count);
  virtual void skip(size_t n);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
  virtual void rewind() { Channel::Reader::rewind(); }

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Reader() {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GATHER_H



