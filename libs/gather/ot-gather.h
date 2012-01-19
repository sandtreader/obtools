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

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Types
typedef unsigned char data_t;
typedef unsigned long length_t;

//==========================================================================
// Individual segment of a buffer
struct Segment
{
  data_t *owned_data;      // Original allocated block, if we allocated it
  data_t *data;            // Unconsumed data start
  length_t length;         // Unconsumed data length

  //--------------------------------------------------------------------------
  // Constructors
  // Default
  Segment(): owned_data(0), data(0), length(0) {}

  // Reference to external data
  Segment(data_t *_data, length_t _length): 
    owned_data(0), data(_data), length(_length) {}

  // Reference to data allocated here
  Segment(length_t _length): 
    owned_data(new data_t[_length]), data(owned_data), length(_length) {}

  // Construct from segment
  Segment(const Segment& s);

  //--------------------------------------------------------------------------
  // Consume N bytes from the beginning
  void consume(length_t n) { data+=n; length-=n; }

  //--------------------------------------------------------------------------
  // Reset to new length
  void reset(length_t n=0) { length=n; }

  //--------------------------------------------------------------------------
  // Destroy - note, explicit call, not destructor
  void destroy() { if (owned_data) delete[] owned_data; owned_data=0; }

  //--------------------------------------------------------------------------
  // Assignment operator, with owned_data copying
  Segment& operator=(const Segment& s);

  //--------------------------------------------------------------------------
  // Take the data from another segment, clearing any owned_data in the
  // original
  Segment& take(Segment& s)
  { owned_data = s.owned_data; s.owned_data = 0;
    data = s.data; length = s.length; return *this; }

  //--------------------------------------------------------------------------
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
private:
  Segment *segments;
  unsigned int count;
  Segment *segment;
  length_t pos;

public:
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t difference_type;
  typedef data_t value_type;
  typedef data_t& reference;
  typedef data_t *pointer;

  BufferIterator() {}
  BufferIterator(Segment *_segments, unsigned int _count,
                 Segment *_segment, length_t _pos):
    segments(_segments), count(_count), segment(_segment), pos(_pos)
  {}
  BufferIterator(const BufferIterator& bi):
    segments(bi.segments), count(bi.count), segment(bi.segment),
    pos(bi.pos)
  {}

  bool operator==(const BufferIterator& bi) const
  {
    return segments == bi.segments &&
           count == bi.count &&
           segment == bi.segment &&
           pos == bi.pos;
  }
  bool operator!=(const BufferIterator& bi) const
  {
    return !operator==(bi);
  }

  BufferIterator& operator++()
  {
    if (++pos >= segment->length)
    {
      ++segment;
      pos = 0;
    }
    return *this;
  }

  BufferIterator& operator+=(Gather::length_t add)
  {
    while ((pos += add) >= segment->length)
    {
      add = pos - segment->length;
      ++segment;
      pos = 0;
    }
    return *this;
  }

  BufferIterator operator+(Gather::length_t add)
  {
    BufferIterator added = *this;
    added += add;
    return added;
  }

  BufferIterator& operator--()
  {
    if (pos)
      --pos;
    else
    {
      if (segment > segments)
      {
        --segment;
        pos = segment->length - 1;
      }
    }
    return *this;
  }

  reference operator*()
  {
    return segment->data[pos];
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

public:
  typedef BufferIterator iterator;

  static const int DEFAULT_SIZE = 4;

  //--------------------------------------------------------------------------
  // Constructor
  Buffer(unsigned int _size = DEFAULT_SIZE): 
    size(_size), count(0), segments(new Segment[_size]) {}

  //--------------------------------------------------------------------------
  // Get count of used segments
  unsigned int get_count() const { return count; }

  //--------------------------------------------------------------------------
  // Get total number of segments allocated
  unsigned int get_size() const { return size; }

  //--------------------------------------------------------------------------
  // Get segment list
  const Segment *get_segments() const { return segments; }

  //--------------------------------------------------------------------------
  // Get total length of data in buffer
  length_t get_length() const;

  //--------------------------------------------------------------------------
  // Resize the buffer to the given number of segments
  void resize(unsigned int new_size);

  //--------------------------------------------------------------------------
  // Reset the buffer to be empty
  void reset();

  //--------------------------------------------------------------------------
  // Limit the buffer to hold at most a particular length of data
  length_t limit(length_t length);

  //--------------------------------------------------------------------------
  // Add a segment at the end, extending if required
  // Segment is shallow copied (owned_data not copied)
  // Returns the added segment
  Segment& add(Segment& seg);

  //--------------------------------------------------------------------------
  // Add a segment from external data to the end of the buffer
  // Returns the added segment
  Segment& add(data_t *data, length_t length);

  //--------------------------------------------------------------------------
  // Add a segment with allocated data to the end of the buffer
  // Returns the added segment
  Segment& add(length_t length);

  //--------------------------------------------------------------------------
  // Add another buffer to the end of this one, copying any owned_data
  void add(const Buffer& buffer);

  //--------------------------------------------------------------------------
  // Add references to a run of data from another buffer
  void add(const Buffer& buffer, length_t offset, length_t len);

  //--------------------------------------------------------------------------
  // Insert a segment at the given index (default 0, the beginning), 
  // extending if required
  // Returns the added segment
  Segment& insert(const Segment& seg, unsigned int pos);

  //--------------------------------------------------------------------------
  // Insert a segment from external data at a given position (default 0, start)
  // Returns the inserted segment
  Segment& insert(data_t *data, length_t length, unsigned int pos=0)
  { return insert(Segment(data, length), pos); }

  //--------------------------------------------------------------------------
  // Insert a segment with allocated data at a given position (default 0,start)
  // Returns the inserted segment
  Segment& insert(length_t length, unsigned int pos=0)
  { return insert(Segment(length), pos); }

  //--------------------------------------------------------------------------
  // Consume N bytes of data from the front of the buffer
  void consume(length_t n);

  //--------------------------------------------------------------------------
  // Copy some data to a contiguous buffer
  length_t copy(data_t *data, length_t offset, length_t len) const;

#if !defined(__WIN32__)
  //--------------------------------------------------------------------------
  // Fill an iovec array with the data
  // iovec must be pre-allocated to the maximum segments of the buffer (size)
  // Returns the number of segments filled in, or size+1 if it overflowed
  unsigned int fill(struct iovec *iovec, unsigned int size);
#endif

  //--------------------------------------------------------------------------
  // Dump the buffer to the given stream, optionally with data as well
  void dump(ostream& sout, bool show_data=false) const;

  //--------------------------------------------------------------------------
  // Iterator functions
  iterator begin() const
  {
    return BufferIterator(segments, count, segments, 0);
  }

  iterator end() const
  {
    return BufferIterator(segments, count, &segments[count], 0);
  }

  //--------------------------------------------------------------------------
  // Destructor
  ~Buffer();
};

//==========================================================================
// Gather reader
class Reader: public Channel::Reader
{
private:
  const Buffer& buffer;

public:
  //------------------------------------------------------------------------
  // Constructor
  Reader(const Buffer& _buffer):
    buffer(_buffer)
  {}

  //--------------------------------------------------------------------------
  // Read implementations - see ot-chan.h for details
  virtual size_t basic_read(void *buf, size_t count) throw (Channel::Error);
  virtual void skip(size_t n) throw (Channel::Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Channel::Error);
  virtual void rewind() throw (Channel::Error) { Channel::Reader::rewind(); }

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Reader() {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GATHER_H



