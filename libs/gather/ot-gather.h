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
#include <ostream>

namespace ObTools { namespace Gather { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Types
typedef unsigned char data_t;
typedef uint32_t length_t;

//==========================================================================
// Individual segment of a buffer
struct Segment
{
  data_t *data;
  length_t length;
  bool owned;         // If set, we allocated it and can deallocate

  //--------------------------------------------------------------------------
  // Constructors
  // Default
  Segment(): data(0), length(0), owned(false) {}

  // Reference to external data
  Segment(data_t *_data, length_t _length): 
    data(_data), length(_length), owned(false) {}

  // Reference to data allocated here
  Segment(length_t _length): 
    data(new data_t[_length]), length(_length), owned(true) {}

  //--------------------------------------------------------------------------
  // Destroy - note, explicit call, not destructor
  void destroy() { if (owned) delete[] data; }
};

//==========================================================================
// Gather buffer 
class Buffer
{
private:
  unsigned int size;      // Size allocated
  unsigned int count;     // Number of segments used
  Segment *segments;      // Allocated array of segments

  // Internals
  void extend();
  Segment& add(const Segment& seg);
  Segment& insert(const Segment& seg, unsigned int pos=0);

public:
  static const int DEFAULT_SIZE = 4;

  //--------------------------------------------------------------------------
  // Constructor
  Buffer(unsigned int _size = DEFAULT_SIZE): 
    size(_size), count(0), segments(new Segment[_size]) {}

  //--------------------------------------------------------------------------
  // Add a segment from external data to the end of the buffer
  // Returns the added segment
  Segment& add(data_t *data, length_t length)
  { return add(Segment(data, length)); }

  //--------------------------------------------------------------------------
  // Add a segment with allocated data to the end of the buffer
  // Returns the added segment
  Segment& add(length_t length)
  { return add(Segment(length)); }

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
  // Dump the buffer to the given stream, optionally with data as well
  void dump(ostream& sout, bool show_data=false);

  //--------------------------------------------------------------------------
  // Destructor
  ~Buffer();
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GATHER_H



