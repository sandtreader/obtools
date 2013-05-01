//==========================================================================
// ObTools::Misc: range.cc
//
// Implementation of RangeSet
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

// Data structure:
//   List of ranges, each comprising start and length
//   Stored in order
//   No overlaps
//   No two ranges touch - always amalgamated before insertion
//   => Set is always optimal

#include "ot-misc.h"
#include "ot-text.h"
#include "ot-xml.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Read from a comma-delimited range string
// e.g. 1-100,110,120,200-1000
// Total length is not set
void UInt64RangeSet::read(const string& text)
{
  // Split into comma-delimited chunks
  vector<string> ranges = Text::split(text);

  for(vector<string>::iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    string range = *p;
    if (range.empty()) continue;

    // Split again on '-'
    vector<string> ends = Text::split(range, '-');

    offset_t start = Text::stoi(ends[0]);
    length_t length = 1;

    // Two ends?
    if (ends.size() >= 2) length = Text::stoi64(ends[1])-start+1;

    insert(start, length);
  }
}

//------------------------------------------------------------------------
// Convert to a comma-delimited string
// e.g. 1-100,110,120,200-1000
// Total length is not recorded
string UInt64RangeSet::str() const
{
  string s;

  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    if (!s.empty()) s+=",";

    s+=Text::i64tos(r.start);

    if (r.length>1)
    {
      s+='-';
      s+=Text::i64tos(r.start+r.length-1);
    }
  }

  return s;
}

//------------------------------------------------------------------------
// Read from XML - reads <range start="x" length="y"/> elements
// (or other element name if provided) from given parent element.
// Ranges may overlap and will be optimised
// together.  Also reads total_length attribute of parent if present
void UInt64RangeSet::read_from_xml(const XML::Element& parent,
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
void UInt64RangeSet::add_to_xml(XML::Element& parent,
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
void UInt64RangeSet::read(Channel::Reader& chan) throw(Channel::Error)
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
void UInt64RangeSet::write(Channel::Writer& chan) const throw(Channel::Error)
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
void UInt64RangeSet::dump(ostream& sout) const
{
  for(list<Range>::const_iterator p = ranges.begin(); p!=ranges.end(); ++p)
  {
    const Range& r = *p;
    sout << r.start << ", " << r.length << endl;
  }
}

//------------------------------------------------------------------------
// << operator to write RangeSet to ostream, in short text form
ostream& operator<<(ostream& s, const UInt64RangeSet& rs)
{
  s << rs.str();
  return s;
}

}} // namespaces
