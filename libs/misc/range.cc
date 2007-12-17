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
#include "ot-xml.h"
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
  // Catch daftness which would otherwise break us
  if (!length) return;

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
// Remove a range
void RangeSet::remove(RangeSet::off_t start, RangeSet::len_t length)
{
  // Catch daftness which would otherwise break us
  if (!length) return;

  off_t end = start+length;

  // Find the first range that ends after or at the start point
  list<Range>::iterator p;

  for (p=ranges.begin(); p!=ranges.end() && p->end() <= start; p++)
    ;

  if (p == ranges.end()) return;  // All end before - nothing to do

  // Now check every range until we reach one that starts after our end
  while (p!=ranges.end() && p->start < end)
  {
    list<Range>::iterator q=p++;  // Protect from deletion
    off_t q_start = q->start;
    off_t q_end = q->end();

    // Check for various cases:
    // (1) Range entirely covered by deletion
    if (start <= q_start && end >= q_end)
    {
      // Remove it entirely
      ranges.erase(q);
    }
    // (2) Range truncated right by start
    else if (end >= q_end)
    {
      // Adjust length 
      q->length = start-q_start;
    }
    // (3) Range truncated left by end
    else if (start <= q_start)
    {
      // Shift start and adjust length
      q->length -= (end-q_start);
      q->start = end;

      // Stop here
      break;
    }
    // (4) Worst case - deletion is in the middle of the range
    else
    {
      // Truncate the current one to hold the left-hand end
      q->length = start-q_start;

      // Create a new one to hold the right-hand end, inserted after
      // the current one, or at end
      ranges.insert(p, Range(end, q_end-end));

      // Stop here
      break;
    }
  }
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
// Read from XML - reads <range start="x" length="y"/> elements
// (or other element name if provided) from given parent element.  
// Ranges may overlap and will be optimised
// together.  Also reads total_length attribute of parent if present
void RangeSet::read_from_xml(const XML::Element& parent, 
			     const string& element_name)
{
  total_length = parent.get_attr_int64("total_length");
  for(XML::Element::const_iterator p(parent.get_children(element_name));p;++p)
  {
    const XML::Element& range = *p;
    insert(range.get_attr_int64("start"), range.get_attr_int64("length"));
  }
}

//------------------------------------------------------------------------
// Convert to XML
// Adds <range start="x" length="y"/> elements to the given XML element
// or other element name if provided
// and adds total_length attribute to parent
void RangeSet::add_to_xml(XML::Element& parent, 
			  const string& element_name) const
{
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    XML::Element& re = parent.add(element_name);
    re.set_attr_int64("start", r.start);
    re.set_attr_int64("length", r.length);
  }

  parent.set_attr_int64("total_length", total_length);
}

//------------------------------------------------------------------------
// Read as binary from a channel;  format as below
void RangeSet::read(Channel::Reader& chan) throw(Channel::Error)
{
  total_length = chan.read_nbo_64();
  uint32_t n = chan.read_nbo_32();
  while (n--)
  {
    uint64_t start = chan.read_nbo_64();
    uint64_t length = chan.read_nbo_64();
    insert(start, length);
  }
}

//------------------------------------------------------------------------
// Write as binary to a channel
// Format is 64-bit total length, then 4-byte count of entries, 
// then alternating 64-bit offset, length
// All values NBO
void RangeSet::write(Channel::Writer& chan) const throw(Channel::Error)
{
  chan.write_nbo_64(total_length);
  chan.write_nbo_32(ranges.size());
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    chan.write_nbo_64(r.start);
    chan.write_nbo_64(r.length);
  }
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
