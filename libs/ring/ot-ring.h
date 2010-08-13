//==========================================================================
// ObTools::Ring: ot-ring.h
//
// Public definitions for ObTools::Ring
//
// Lock-free ring-buffer template
//
// Allows a single writer and single reader to communicate over a fixed-size
// message queue without using locks/mutexes
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_RING_H
#define __OBTOOLS_RING_H

namespace ObTools { namespace Ring { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Ring buffer template 

// Template parameters:
//   ITEM_T:       Type of an item in the ring buffer

template<class ITEM_T> class Buffer
{
private:
  ITEM_T *items;            // Fixed-size, preallocated array
  int length;               // Size of array
  int in_index;             // Index of next item to be written
  int out_index;            // Index of next item to be read

  // Note: in_index == out_index => queue empty
  //       in_index == out_index-1 (mod length) => queue full
  // Therefore queue is full at (length-1) items, and we allocate one
  // more than asked for

  //--------------------------------------------------------------------------
  // Modular increment - increments the value given, mod length
  int inc(int n) { return (++n>=length)?0:n; }
  
public:
  //--------------------------------------------------------------------------
  // Constructor
  Buffer(int _length): items(new ITEM_T[_length+1]), length(_length+1),
    in_index(0), out_index(0) {}

  //--------------------------------------------------------------------------
  // Write an item - returns whether successfully written (buffer wasn't full)
  bool put(const ITEM_T& item)
  {
    int next_in_index = inc(in_index);
    if (next_in_index == out_index) return false;
    items[in_index] = item;
    in_index = next_in_index;
    return true;
  }

  //--------------------------------------------------------------------------
  // Read an item - returns whether successfully fetched (buffer wasn't
  // empty) and sets item_p if so
  bool get(ITEM_T& item_p)
  {
    if (out_index == in_index) return false;
    item_p = items[out_index];
    out_index = inc(out_index);
    return true;
  }

  //--------------------------------------------------------------------------
  // Destructor
  ~Buffer()
  {
    delete[] items;
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_RING_H



