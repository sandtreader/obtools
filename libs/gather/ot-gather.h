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
  int size;               // Size allocated
  int count;              // Number of segments used
  Segment *segments;      // Allocated array of segments

  // Internals
  void extend();
  Segment& add_segment(const Segment& seg);

public:
  //--------------------------------------------------------------------------
  // Constructor
  Buffer(int _size): 
    size(_size), count(0), segments(new Segment[_size]) {}

  //--------------------------------------------------------------------------
  // Add a segment from external data
  // Returns the added segment
  Segment& add_segment(data_t *data, length_t length)
  { return add_segment(Segment(data, length)); }

  //--------------------------------------------------------------------------
  // Add a segment with allocated data
  // Returns the added segment
  Segment& add_segment(length_t length)
  { return add_segment(Segment(length)); }

  //--------------------------------------------------------------------------
  // Destructor
  ~Buffer();
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_GATHER_H



