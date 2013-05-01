//==========================================================================
// ObTools::Misc: ot-misc.h
//
// Public definitions for ObTools::Misc
// Miscellaneous useful classes
// 
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_MISC_H
#define __OBTOOLS_MISC_H

#include <string>
#include <map>
#include <list>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "ot-xml.h"
#include "ot-chan.h"
#include "ot-text.h"

namespace ObTools { namespace Misc { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
//MD5 sum class
class MD5
{
private:
  // State taken from MD5Context
  uint32_t ctx_buf[4];
  uint32_t ctx_bits[2];
  char ctx_in[64];

  void byte_reverse(char *buf, unsigned longs);
  void transform(uint32_t buf[4], uint32_t in[16]);

public:
  //------------------------------------------------------------------------
  // Constructor
  MD5() { initialise(); }

  //------------------------------------------------------------------------
  // Reinitialise before re-use
  void initialise();

  //------------------------------------------------------------------------
  // Update to reflect the concatenation of another buffer full
  // of bytes. 
  void update(const char *buf, unsigned len);

  //------------------------------------------------------------------------
  // Final wrapup
  // Fills 'digest' with 16 binary bytes representing the MD5 sum 
  void finalise(unsigned char digest[16]);

  //--------------------------------------------------------------------------
  // C++ friendly version: MD5 sum a string (returns hex string)
  // Re-initialises each time, so safe to reuse
  string sum(const string& text);

  //------------------------------------------------------------------------
  // Destructor 
  ~MD5();
};

//==========================================================================
//CRC calculator class
//Various 16-bit CRC algorithms - note only single block at a time
//Note:  This is a minefield!
//There seems general agreement about the parameters of CRC16, the only
//difference in implementation comes from whether it is reflected or not,
//which probably indicates a hardware or software origin.

//But there is a long-running argument about exactly what CCITT is 'supposed'
//to be.  The most prevelant is probably the plain CCITT below; with
//CCITT_ZERO the next - but I'm inclined to agree with Joe Geluso 
//that CCITT_MOD may be the 'right' answer, so all three are offered!

//Some CCITT versions flip the result; CRC-16 never does

//References:
//   http://www.ross.net/crc/crcpaper.html
//   http://www.joegeluso.com/software/articles/ccitt.htm
// and
//   http://www.zorn.breitbandkatze.de/crc.html

class CRC
{
private:
  uint16_t combinations[256];

public:
  typedef uint16_t crc_t;

  enum Algorithm
  {
    ALGORITHM_CRC16 = 0,       // x^16 + x^15 + x^2 + 1, init 0x0000 
    ALGORITHM_CCITT = 1,       // x^16 + x^12 + x^5 + 1, init 0xFFFF
    ALGORITHM_CCITT_ZERO = 2,  // x^16 + x^12 + x^5 + 1, init 0x0000 
    ALGORITHM_CCITT_MOD  = 3   // x^16 + x^12 + x^5 + 1, init 0x1DOF
  } algorithm;

  bool reflected;  // If clear, software-like with top bit first,
                   // If set, network-like with low bit first
  bool flip;       // If set, flips (^0xFFFF) the result

  //------------------------------------------------------------------------
  // Constructor (creates internal combination table, relatively expensive)
  CRC(Algorithm _alg = ALGORITHM_CRC16, bool _reflected = false,
      bool _flip = false);

  //------------------------------------------------------------------------
  // Calculate new CRC for a block
  crc_t calculate(const unsigned char *data, size_t length);

  //------------------------------------------------------------------------
  // Calculate a CRC for a string (can be binary)
  crc_t calculate(const string& data);
};

//==========================================================================
// CRC32 calculator class
// There isn't much argument about this one - this is the V.42 version
// as used in PNG, Ethernet etc.
class CRC32
{
private:
  uint32_t combinations[256];

public:
  typedef uint32_t crc_t;

  enum Algorithm
  {
    ALGORITHM_CRC32 = 0,       // 0x04c11db7, init 0xFFFFFFFF
    ALGORITHM_CRC32C = 1,      // 0x1edc6f41, init 0xFFFFFFFF
  } algorithm;

  bool reflected;  // If clear, software-like with top bit first,
                   // If set, network-like with low bit first
  bool flip;       // If set, flips (^0xFFFFFFFF) the result

  //------------------------------------------------------------------------
  // Constructor (creates internal combination table, relatively expensive)
  CRC32(Algorithm _alg = ALGORITHM_CRC32, bool _reflected = true,
        bool _flip = true);

  //------------------------------------------------------------------------
  // Calculate new CRC for a block
  crc_t calculate(const unsigned char *data, size_t length);

  //------------------------------------------------------------------------
  // Calculate a CRC for a string (can be binary)
  crc_t calculate(const string& data);
};


//==========================================================================
//Propertylist class
//Simple sugaring of a string-string map
class PropertyList: public map<string, string>
{
public:
  //------------------------------------------------------------------------
  // Constructor
  PropertyList() {}

  //------------------------------------------------------------------------
  // Constructor from delimited string - e.g. foo=1,bar=2
  PropertyList(const string& str, char sep=',', char quote='"');

  //------------------------------------------------------------------------
  // Copy constructor and assignment operator from map<string, string>
  PropertyList(const map<string, string>& o) 
  { *(map<string, string> *)this = o; }
  PropertyList& operator=(const map<string, string>& o) 
  { *(map<string, string> *)this = o; return *this; }

  //--------------------------------------------------------------------------
  // Add a value
  void add(const string& name, const string& value)
  { erase(name); insert(make_pair(name,value)); }  //Note, not using []

  //--------------------------------------------------------------------------
  // Add a integer value
  void add(const string& name, int value)
  { 
    ostringstream oss;
    oss << value;
    add(name, oss.str());
  }

  //--------------------------------------------------------------------------
  // Add a boolean value
  // Note: Explicitly typed to avoid horror where literal strings are cast
  // to bool rather than std::string
  void add_bool(const string& name, bool value)
  { add(name, value?"true":"false"); }

  //--------------------------------------------------------------------------
  // Check if a named property exists
  bool has(const string& name) const
  {
    return find(name) != end();
  }

  //--------------------------------------------------------------------------
  // Get a value, with default
  string get(const string& name, const string& def="") const
  {
    const_iterator p = find(name);
    if (p!=end()) return p->second;
    return def;
  }

  //--------------------------------------------------------------------------
  // Get an integer value, with default
  int get_int(const string& name, int def=0) const
  {
    string v = get(name);
    if (!v.empty()) return atoi(v.c_str());
    return def;
  }

  //--------------------------------------------------------------------------
  // Get a boolean value, with default
  // Anything beginning [TtYy] is assumed to be 'true', anything else 'false'
  bool get_bool(const string& name, bool def=false) const
  {
    string v = get(name);
    if (!v.empty())
    {
      switch(v[0])
      {
	case 'T': case 't':
	case 'Y': case 'y':
	  return true;

	default:
	  return false;
      }
    }

    return def;
  }

  //--------------------------------------------------------------------------
  // Get an real value, with default
  double get_real(const string& name, double def=0.0) const
  {
    string v = get(name);
    if (!v.empty()) return atof(v.c_str());
    return def;
  }

  //--------------------------------------------------------------------------
  // Overridden read-only [] operator
  // Unlike standard [], doesn't create a blank value if it doesn't exist
  string operator[](const string& name) const { return get(name); }

  //------------------------------------------------------------------------
  // Variable interpolation of property list into a string
  // Replaces (e.g.) $var with value from property list
  // Variables are terminated by non-alphanum or ";"
  // $ and ; can be escaped as $$ and $;
  // e.g.
  //  $name   -> fred
  //  $name;s -> freds
  //  $name$name -> fredfred
  //  $name$;s -> fred;s
  //  $$name  -> $fred
  // Unset variables are not substituted
  string interpolate(const string& text);

  //--------------------------------------------------------------------------
  // Dump contents
  void dump(ostream& s, const string& prefix="    ", 
	    const string& separator=" = ") const;

  //--------------------------------------------------------------------------
  // Convert to delimited string
  string str(char sep=',', char quote='"');

};

//------------------------------------------------------------------------
// << operator to write PropertyList to ostream
ostream& operator<<(ostream& s, const PropertyList& pl);

//==========================================================================
//Random string generator
//Uses best available random number source (e.g. /dev/urandom), falls
//back to a Marsaglia MWC in its absence
class Random
{
private:
  uint32_t w, z;     // Seed state
  int count;         // Number of calls to generate_binary

public:
  //------------------------------------------------------------------------
  // Constructor
  Random();

  //------------------------------------------------------------------------
  // Get random binary bytes up to N bytes long
  void generate_binary(unsigned char *p, int n);

  //------------------------------------------------------------------------
  // Get a random hex string up to N bytes (N*2 hex digits) long
  string generate_hex(int n);

  //------------------------------------------------------------------------
  // Get a random 32-bit number
  uint32_t generate_32();

  //------------------------------------------------------------------------
  // Get a random 64-bit number
  uint64_t generate_64();

  //------------------------------------------------------------------------
  // Get a random number in the range 0 and (n-1)
  unsigned int generate_up_to(unsigned int n);
};

//==========================================================================
//Hex data dumper
class Dumper
{
  ostream& sout;
  int width;
  int split;
  bool ascii;

public:
  //------------------------------------------------------------------------
  // Constructor
  // width gives number of bytes per line
  // split is interval of spaces between bytes: 0 is no spaces
  // ascii gives printable ASCII listing as well if set
  Dumper(ostream& _sout, int _width=16, int _split=4, bool _ascii=true): 
    sout(_sout), width(_width), split(_split), ascii(_ascii) {}

  //------------------------------------------------------------------------
  // Dump a block
  void dump(const void *block, int length);

  //------------------------------------------------------------------------
  // Dump a string
  void dump(const string& s) { dump(s.data(), s.size()); }
};

//==========================================================================
//Range set - stores a list of integer ranges - e.g. file fragments, and
//offers various useful operations on them (range.cc)
template<typename T, typename L>
class RangeSet
{
public:
  // Offset and length typedefs
  typedef T offset_t;
  typedef L length_t;

  // Internal record of a single range
  struct Range
  {
    offset_t start;
    length_t length;
    Range(offset_t _start, length_t _length):
      start(_start), length(_length)
    {}

    offset_t end() const { return start+length; }  // One past the end

    bool operator==(const Range& b) const
    { return start == b.start && length == b.length; }
  };

  // List of ranges, stored in order
  list<Range> ranges;

  // Expected end offset - provides information for various operations 
  // below.  Will be modified by insertions if exceeded - hence 0 is fine
  // if you want just to count the total length seen so far
  offset_t end_offset;

  // Iterator types
  typedef typename list<Range>::iterator iterator;
  iterator begin() { return ranges.begin(); }
  iterator end()   { return ranges.end();   }
  typedef typename list<Range>::const_iterator const_iterator;
  const_iterator begin() const { return ranges.begin(); }
  const_iterator end() const   { return ranges.end();   }

  //------------------------------------------------------------------------
  // Constructor
  RangeSet(offset_t _end_offset=offset_t()): end_offset(_end_offset) {}

  //------------------------------------------------------------------------
  // Insert a range
  // Optimises against adjacent ranges, overlaps allowed
  void insert(offset_t start, length_t length=1)
  {
    // Catch daftness which would otherwise break us
    if (!length) return;

    offset_t end = start+length;

    // Find the first range which starts after or at the new one
    typename list<Range>::iterator prev_p = ranges.end();
    typename list<Range>::iterator next_p;
    typename list<Range>::iterator new_p;

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
    offset_t& new_start  = new_p->start;
    length_t& new_length = new_p->length;

    // Now work forward from new_p 'eating' ranges we overlap with or touch
    for(next_p = new_p, ++next_p;
        next_p != ranges.end();)
    {
      typename list<Range>::iterator this_p = next_p++; // Protect from deletion
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

    // Check if end offset exceeded
    if (end>end_offset) end_offset = end;
  }

  //------------------------------------------------------------------------
  // Insert a range set (inserts every element = set union)
  void insert(const RangeSet& o)
  {
    for(typename list<Range>::const_iterator
        p = o.ranges.begin(); p!=o.ranges.end(); ++p)
    {
      const Range& r = *p;
      insert(r.start, r.length);
    }
  }

  //------------------------------------------------------------------------
  // Set union and addition operator
  RangeSet set_union(const RangeSet& o) const
  { RangeSet r=*this; r.insert(o); return r; }

  RangeSet operator+(const RangeSet& o) const { return set_union(o); }

  //------------------------------------------------------------------------
  // Remove a range
  void remove(offset_t start, length_t length=1)
  {
    // Catch daftness which would otherwise break us
    if (!length) return;

    offset_t end = start+length;

    // Find the first range that ends after or at the start point
    typename list<Range>::iterator p;

    for (p=ranges.begin(); p!=ranges.end() && p->end() <= start; p++)
      ;

    if (p == ranges.end()) return;  // All end before - nothing to do

    // Now check every range until we reach one that starts after our end
    while (p!=ranges.end() && p->start < end)
    {
      typename list<Range>::iterator q=p++;  // Protect from deletion
      offset_t q_start = q->start;
      offset_t q_end = q->end();

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
  // Remove a range set (removes every element = set difference)
  void remove(const RangeSet& o)
  {
    for(typename list<Range>::const_iterator
        p = o.ranges.begin(); p!=o.ranges.end(); ++p)
    {
      const Range& r = *p;
      remove(r.start, r.length);
    }
  }

  //------------------------------------------------------------------------
  // Set difference, and subtraction operator
  RangeSet difference(const RangeSet& o) const
  { RangeSet r=*this; r.remove(o); return r; }

  RangeSet operator-(const RangeSet& o) { return difference(o); }

  //------------------------------------------------------------------------
  // Return a new set of all the 'holes' in the set, up to the end offset
  RangeSet inverse() const
  {
    RangeSet inverse;
    const Range *last = 0;

    // Walk filling in gaps between last end (or 0) and new start
    for(typename list<Range>::const_iterator
        p = ranges.begin(); p!=ranges.end(); ++p)
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
      if (end_offset > last->end())
        inverse.ranges.push_back(Range(last->end(), end_offset-last->end()));
    }
    else if (end_offset > 0)
    {
      // Empty set - add whole range
      inverse.ranges.push_back(Range(0, end_offset));
    }

    // Total length is the same as ours
    inverse.end_offset = end_offset;
    return inverse;
  }

  //------------------------------------------------------------------------
  // Return a new set which is the intersection of this set with another
  // and ^ operator
  RangeSet intersection(const RangeSet& o) const
  {
    // Ensure the other set extends to the same size as we do
    RangeSet o2 = o;
    o2.end_offset = end_offset;

    // Get the inverse of the other set, to our full length
    RangeSet io2 = o2.inverse();

    // Subtract this from us - thus removing anything that is not in o
    RangeSet r = *this;
    r.remove(io2);
    return r;
  }

  RangeSet operator^(const RangeSet& o) const { return intersection(o); }

  //------------------------------------------------------------------------
  // Return a new set which is the intersection of all given sets
private:
  struct IntersectState
  {
    const_iterator it;
    bool end;
    const_iterator list_end;

    IntersectState(const const_iterator& _it, const const_iterator& _list_end):
      it(_it), end(false), list_end(_list_end)
    {}

    bool operator<(const IntersectState& b) const
    {
      return (end ? it->end() : it->start)
              < (b.end ? b.it->end() : b.it->start);
    }
  };

public:
  static RangeSet intersection(const list<RangeSet>& sets)
  {
    list<IntersectState> state;
    for (typename list<RangeSet>::const_iterator
         it = sets.begin(); it != sets.end(); ++it)
    {
      state.push_back(IntersectState(it->begin(), it->end()));
    }

    RangeSet result;
    unsigned int count(0);
    T current_start;

    while (true)
    {
      typename list<IntersectState>::iterator first = state.end();
      for (typename list<IntersectState>::iterator
           it = state.begin(); it != state.end(); ++it)
      {
        if (it->it != it->list_end &&
            (first == state.end() || *it < *first))
        {
          first = it;
        }
      }
      if (first == state.end())
        break;

      if (first->end)
      {
        if (count == state.size())
          result.insert(current_start, first->it->end() - current_start);
        --count;
        ++(first->it);
        first->end = false;
      }
      else
      {
        ++count;
        first->end = true;
        if (count == state.size())
          current_start = first->it->start;
      }
    }

    return result;
  }

  //------------------------------------------------------------------------
  // Clear the set
  void clear() { ranges.clear(); }

  //------------------------------------------------------------------------
  // Check if a given range is all present
  bool contains(offset_t start, length_t length=1) const
  {
    offset_t end = start+length;

    // Because the set is always optimal, there must be a single range
    // which includes both the start and end
    for(typename list<Range>::const_iterator
        p = ranges.begin(); p!=ranges.end(); ++p)
    {
      const Range& r = *p;
      if (start >= r.start && start < r.end() && end <= r.end()) return true;
    }

    return false;
  }

  //------------------------------------------------------------------------
  // Get total coverage of the set (sum of all range lengths)
  length_t coverage() const
  {
    length_t sum = 0;

    // Simply add up all the range lengths
    for(typename list<Range>::const_iterator
        p = ranges.begin(); p!=ranges.end(); ++p)
      sum += p->length;

    return sum;
  }

  //------------------------------------------------------------------------
  // Get number of ranges
  typename list<Range>::size_type count() const { return ranges.size(); }

  //------------------------------------------------------------------------
  // Check if the set is complete up to end_offset, or there was nothing
  // to fetch
  bool is_complete() const { return !end_offset||contains(0, end_offset); }
  //------------------------------------------------------------------------
  // Get percentage coverage
  int percentage_complete() 
  { return end_offset?(100*coverage()/end_offset):100; }

  //------------------------------------------------------------------------
  // Show the set as a string 'fuel gauge' of the given max string length
  // Each character in the string maps to a fractional part of the range
  // (measured to end_offset):
  //   ' ' (space):  No range present in this fraction
  //   '-'        :  Part of range present in this fraction
  //   '='        :  Full range present in this fraction
  // Most simply, this gives a fuel gauge for monotonically increasing
  // ranges, but also allows for insertion at multiple points
  // If length is more than the total length of the range, it is reduced to it
  // (a character can never represent less than 1 unit)
  string gauge(unsigned int length=50) const
  {
    typename list<Range>::const_iterator p = ranges.begin();
    string s;

    if (length > end_offset) length = end_offset;

    for(unsigned int i=0; i<length; i++)
    {
      // Calculate (exactly, avoiding rounding and overflow) start and end 
      // of this fraction (end=start of next)
      double this_start = (double)end_offset*i/(double)length;
      double this_end   = (double)end_offset*(i+1)/(double)length;

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
};

class UInt64RangeSet: public RangeSet<uint64_t, uint64_t>
{
public:
  //------------------------------------------------------------------------
  // Constructor
  UInt64RangeSet(length_t end_offset=0):
    RangeSet<uint64_t, uint64_t>(end_offset) {}

  //------------------------------------------------------------------------
  // Copy Constructor
  UInt64RangeSet(const RangeSet<uint64_t, uint64_t>& rs):
    RangeSet<uint64_t, uint64_t>(rs) {}

  //------------------------------------------------------------------------
  // Constructor from string
  UInt64RangeSet(const string& text, length_t end_offset=0):
    RangeSet<uint64_t, uint64_t>(end_offset) { read(text); }

  //------------------------------------------------------------------------
  // Read from a comma-delimited range string
  // e.g. 1-100,110,120,200-1000
  // Total length is not set
  void read(const string& text);

  //------------------------------------------------------------------------
  // Convert to a comma-delimited string
  // e.g. 1-100,110,120,200-1000
  // Total length is not recorded
  string str() const;

  //------------------------------------------------------------------------
  // Read from XML - reads <range start="x" length="y"/> elements
  // (or other element name if provided) from given parent element.
  // Ranges may overlap and will be optimised
  // together.  Also reads end_offset attribute of parent if present
  void read_from_xml(const XML::Element& parent,
                     const string& element_name="range");

  //------------------------------------------------------------------------
  // Convert to XML
  // Adds <range start="x" length="y"/> elements to the given XML element
  // or other element name if provided
  // and adds end_offset attribute to parent
  void add_to_xml(XML::Element& parent,
                  const string& element_name="range") const;

  //------------------------------------------------------------------------
  // Read as binary from a channel;  format as below
  void read(Channel::Reader& chan) throw(Channel::Error);

  //------------------------------------------------------------------------
  // Write as binary to a channel
  // Format is 4-byte count of entries, then alternating 64-bit offset, length
  // All values NBO
  void write(Channel::Writer& chan) const throw(Channel::Error);

  //------------------------------------------------------------------------
  // Dump the set to the given output, one line per range, in form
  // start, length
  void dump(ostream& sout) const;
};

//------------------------------------------------------------------------
// << operator to write RangeSet to ostream
ostream& operator<<(ostream& s, const UInt64RangeSet& rs);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MISC_H
