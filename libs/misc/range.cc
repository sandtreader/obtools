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
  // Find the first range which starts after or at the new one
  list<Range>::iterator prev_p = ranges.end();
  list<Range>::iterator next_p;
  list<Range>::iterator new_p;

  for (next_p=ranges.begin(); 
       next_p!=ranges.end() && next_p->start < start;
       prev_p = next_p++)
    ;

  // If there was one before, check if it overlaps or touches us
  if (prev_p != ranges.end()
      && prev_p->start + prev_p->length >= start)
  {
    // Extend this one to include this if it's smaller
    if (start+length > prev_p->start+prev_p->length)
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
  if (start+length>total_length) total_length = start+length;
}

//------------------------------------------------------------------------
// Check if a given range is all present
bool RangeSet::contains(off_t start, len_t length) const
{

}

//------------------------------------------------------------------------
// Return a new set of all the 'holes' in the set
RangeSet RangeSet::invert() const
{

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
    // Calculate (exactly, avoiding rounding and overflow) start of this 
    // fraction and start of next
    off_t this_start = (off_t)((double)total_length*i/(double)length);
    off_t next_start = (off_t)((double)total_length*(i+1)/(double)length);

    int contained = 0;  // 0 = none, 1 = some, 2 = all

    while(p!=ranges.end())
    {

      ++p;
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
