//==========================================================================
// ObTools::Misc: range.cc
//
// Implementation of RangeSet
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

// Data structure:
//   List of ranges, each comprising start and length
//   Stored in order
//   No overlaps
//   No two ranges touch - always amalgamated before insertion
//   => Set is always optimal

#include "ot-misc.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Insert a range
// Optimises against adjacent ranges, overlaps allowed
void RangeSet::insert(RangeSet::off_t start, RangeSet::len_t length)
{
  off_t end = start+length;

  // Find the first range which starts after or at the new one
  list<Range>::iterator prev_p = ranges.end();
  list<Range>::iterator next_p;
  list<Range>::iterator new_p;

  for (next_p=ranges.begin(); 
       next_p!=ranges.end() && next_p->start < start;
       prev_p = next_p++)
    ;

  // If there was one before, check if it overlaps or touches us
  if (prev_p != ranges.end() && prev_p->end() >= start)
  {
    // Extend this one to include this if it's smaller
    if (end > prev_p->end())
      prev_p->length = start+length-prev_p->start;

    // Use this one as the one to check for overlaps afterwards
    new_p = prev_p;
  }
  else
  {
    // Not overlapping/touching previous - insert before next, or at 
    // end if none found
    new_p = ranges.insert(next_p, Range(start, length));
  }
  
  // Convenient refs for accessing & modifying new (or extended) range
  off_t& new_start  = new_p->start;
  len_t& new_length = new_p->length;

  // Now work forward from new_p 'eating' ranges we overlap with or touch
  for(next_p = new_p, ++next_p;
      next_p != ranges.end();)
  {
    list<Range>::iterator this_p = next_p++;  // Protect from deletion
    Range& next = *this_p;
    
    // Check for overlap/touch
    if (new_start + new_length >= next.start)
    {
      // Extend new one to include this if it is longer
      if (next.start+next.length > new_start+new_length)
	new_length = next.start+next.length-new_start;

      ranges.erase(this_p);  // Delete eaten one
    }
  }

  // Check if total_length exceeded 
  if (end>total_length) total_length = end;
}

//------------------------------------------------------------------------
// Check if a given range is all present
bool RangeSet::contains(off_t start, len_t length) const
{
  off_t end = start+length;

  // Because the set is always optimal, there must be a single range
  // which includes both the start and end
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    if (start >= r.start && start < r.end() && end <= r.end()) return true;
  }

  return false;
}

//------------------------------------------------------------------------
// Get total coverage of the set (sum of all range lengths)
RangeSet::len_t RangeSet::coverage() const
{
  len_t sum = 0;

  // Simply add up all the range lengths
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
    sum += p->length;

  return sum;
}

//------------------------------------------------------------------------
// Return a new set of all the 'holes' in the set
RangeSet RangeSet::invert() const
{
  RangeSet inverse;
  const Range *last = 0;

  // Walk filling in gaps between last end (or 0) and new start
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    if (last)
      inverse.ranges.push_back(Range(last->end(), r.start-last->end()));
    else if (r.start > 0)
      inverse.ranges.push_back(Range(0, r.start));

    last = &r;
  }

  // Complete with gap (if any) between final end and total length
  if (last)
  {
    if (total_length > last->end())
      inverse.ranges.push_back(Range(last->end(), total_length-last->end()));
  }
  else if (total_length > 0)
  {
    // Empty set - add whole range
    inverse.ranges.push_back(Range(0, total_length));
  }

  // Total length is the same as ours
  inverse.total_length = total_length;
  return inverse;
}

//------------------------------------------------------------------------
// Show the set as a string 'fuel gauge' of the given string length
// Each character in the string maps to a fractional part of the range
// (measured to total_length):
//   ' ' (space):  No range present in this fraction
//   '-'        :  Part of range present in this fraction
//   '='        :  Full range present in this fraction
// Most simply, this gives a fuel gauge for monotonically increasing
// ranges, but also allows for insertion at multiple points
string RangeSet::gauge(unsigned int length) const
{
  list<Range>::const_iterator p = ranges.begin(); 
  string s;

  for(unsigned int i=0; i<length; i++)
  {
    // Calculate (exactly, avoiding rounding and overflow) start and end 
    // of this fraction (end=start of next) 
    double this_start = (double)total_length*i/(double)length;
    double this_end   = (double)total_length*(i+1)/(double)length;

    int contained = 0;  // 0 = none, 1 = some, 2 = all

    while (p!=ranges.end())
    {
      const Range& r = *p;

      // Skip to next fraction if not yet overlapping this range
      if ((double)r.start >= this_end) break;

      // Check if we've already left this range
      if (this_start >= (double)r.end()) 
      { 
	++p;
	continue;
      }

      // So we have an overlap - is it total?
      if ((double)r.start <= this_start && (double)r.end() >= this_end)
	contained = 2;
      else
	contained = 1;

      break;
    }

    s += " -="[contained];
  }

  return s;
}

//------------------------------------------------------------------------
// Dump the set to the given output, one line per range, in form
// start, length
void RangeSet::dump(ostream& sout) const
{
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    sout << r.start << ", " << r.length << endl;
  }
}

//------------------------------------------------------------------------
// << operator to write RangeSet to ostream
ostream& operator<<(ostream& s, const RangeSet& rs) 
{ 
  rs.dump(s); 
  return s;
}

}} // namespaces
