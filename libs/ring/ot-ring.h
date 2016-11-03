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

#include <atomic>
#include <vector>

namespace ObTools { namespace Ring {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Ring buffer template

// Template parameters:
//   ITEM_T:       Type of an item in the ring buffer

template<class ITEM_T> class Buffer
{
private:
  vector<ITEM_T> items;           // Fixed array of items
  atomic<unsigned> in_index{0};   // Index of next item to be written
  atomic<unsigned> out_index{0};  // Index of next item to be read

  // Note: in_index == out_index => queue empty
  //       in_index == out_index-1 (mod length) => queue full
  // Therefore queue is full at (length-1) items, and we allocate one
  // more than asked for

  //------------------------------------------------------------------------
  // Modular increment - increments the value given, mod length
  unsigned inc(unsigned n) { return (++n>=items.size())?0:n; }

public:
  //------------------------------------------------------------------------
  // Constructor
  Buffer(int length): items(length+1) {}

  //------------------------------------------------------------------------
  // Write an item - returns whether successfully written (buffer wasn't full)
  bool put(const ITEM_T& item)
  {
    unsigned next_in_index = inc(in_index);
    if (next_in_index == out_index) return false;
    items[in_index] = item;
    in_index = next_in_index;
    return true;
  }

  //------------------------------------------------------------------------
  // Read an item - returns whether successfully fetched (buffer wasn't
  // empty) and sets item_p if so
  bool get(ITEM_T& item_p)
  {
    if (out_index == in_index) return false;
    item_p = items[out_index];
    out_index = inc(out_index);
    return true;
  }

  // Ways to flush the queue:  Both end up with in_index=out_index, but
  // you must call the right one depending whether you are the putter or
  // getter, otherwise there is a race condition

  //------------------------------------------------------------------------
  // Flush the queue, called from putter side
  void flush_from_put() { in_index = out_index; }

  //------------------------------------------------------------------------
  // Flush the queue, called from getter side
  void flush_from_get() { out_index = in_index; }

  //------------------------------------------------------------------------
  // Get array size
  unsigned size() const { return items.size()-1; }

  //------------------------------------------------------------------------
  // Get number of items used
  unsigned used() const { return out_index<=in_index?
                                    in_index-out_index:
                                    in_index+items.size()-out_index; }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_RING_H



