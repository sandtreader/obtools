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
#include <set>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "ot-xml.h"
#include "ot-chan.h"
#include "ot-text.h"

namespace ObTools { namespace Misc {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// MD5 sum class
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

  //------------------------------------------------------------------------
  // C++ friendly version: MD5 sum a string into byte array
  void sum(const string& text, unsigned char digest[16]);

  //------------------------------------------------------------------------
  // C++ friendly version: MD5 sum a string (returns hex string)
  // Re-initialises each time, so safe to reuse
  string sum(const string& text);

  //--------------------------------------------------------------------------
  // C++ friendly version: MD5 sum a string into Base64
  string sum_base64(const string& text);

  //------------------------------------------------------------------------
  // C++ friendly version: MD5 sum a string, returning combination of digest
  // as an integer (read as two big-endian ints and XOR'ed)
  uint64_t hash_to_int(const string& text);

  //------------------------------------------------------------------------
  // Destructor
  ~MD5();
};

//==========================================================================
// CRC calculator class
// Various 16-bit CRC algorithms - note only single block at a time
// Note:  This is a minefield!
// There seems general agreement about the parameters of CRC16, the only
// difference in implementation comes from whether it is reflected or not,
// which probably indicates a hardware or software origin.

// But there is a long-running argument about exactly what CCITT is 'supposed'
// to be.  The most prevelant is probably the plain CCITT below; with
// CCITT_ZERO the next - but I'm inclined to agree with Joe Geluso
// that CCITT_MOD may be the 'right' answer, so all three are offered!

// Some CCITT versions flip the result; CRC-16 never does

// References:
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

  //------------------------------------------------------------------------
  // Stream-style usage

  //------------------------------------------------------------------------
  // Get the initial value to work with
  crc_t initialiser() const;

  //------------------------------------------------------------------------
  // Consume some data and update CRC
  crc_t consume(const unsigned char *data, size_t length, crc_t crc) const;

  //------------------------------------------------------------------------
  // Finalise CRC
  crc_t finalise(crc_t crc) const;
};


//==========================================================================
// Propertylist class
// Simple sugaring of a string-string map
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
  PropertyList(const map<string, string>& o):
    map<string, string>(o)
  {}
  PropertyList& operator=(const map<string, string>& o)
  { map<string, string>::operator=(o); return *this; }

  //------------------------------------------------------------------------
  // Add a value
  void add(const string& name, const string& value)
  { erase(name); insert(make_pair(name,value)); }  //Note, not using []

  //------------------------------------------------------------------------
  // Add a integer value
  void add(const string& name, int value)
  {
    ostringstream oss;
    oss << value;
    add(name, oss.str());
  }
  void add(const string& name, uint64_t value)
  {
    ostringstream oss;
    oss << value;
    add(name, oss.str());
  }

  //------------------------------------------------------------------------
  // Add a boolean value
  // Note: Explicitly typed to avoid horror where literal strings are cast
  // to bool rather than std::string
  void add_bool(const string& name, bool value)
  { add(name, value?"true":"false"); }

  //------------------------------------------------------------------------
  // Check if a named property exists
  bool has(const string& name) const
  {
    return find(name) != end();
  }

  //------------------------------------------------------------------------
  // Get a value, with default
  string get(const string& name, const string& def="") const
  {
    const_iterator p = find(name);
    if (p!=end()) return p->second;
    return def;
  }

  //------------------------------------------------------------------------
  // Get an integer value, with default
  int get_int(const string& name, int def=0) const
  {
    string v = get(name);
    if (!v.empty()) return atoi(v.c_str());
    return def;
  }

  //------------------------------------------------------------------------
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

  //------------------------------------------------------------------------
  // Get an real value, with default
  double get_real(const string& name, double def=0.0) const
  {
    string v = get(name);
    if (!v.empty()) return atof(v.c_str());
    return def;
  }

  //------------------------------------------------------------------------
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
  string interpolate(const string& text) const;

  //------------------------------------------------------------------------
  // Dump contents
  void dump(ostream& s, const string& prefix="    ",
            const string& separator=" = ") const;

  //------------------------------------------------------------------------
  // Convert to delimited string
  string str(char sep=',', char quote='"');

};

//--------------------------------------------------------------------------
// << operator to write PropertyList to ostream
ostream& operator<<(ostream& s, const PropertyList& pl);

//==========================================================================
// Hash interpolator.  Takes a PropertyList and generates new properties
// based on hashes of interpolated strings from the existing ones.  Used to
// generate pathname / URL rewrites with load-balancing elements
class HashInterpolator
{
public:
  struct Hash
  {
    string name;
    unsigned int modulus;
    string pattern;

    // Comparator for testing
    bool operator==(const Hash& o) const
    { return name==o.name && modulus==o.modulus && pattern==o.pattern; }

    Hash(const string& _name, unsigned int _modulus, const string& _pattern):
      name(_name), modulus(_modulus), pattern(_pattern) {}
  };

private:
  list<Hash> hashes;

public:
  //------------------------------------------------------------------------
  // Default constructor, no hashes
  HashInterpolator() {}

  //------------------------------------------------------------------------
  // Constructor from XML, reads <hash> elements from given root e.g.
  //  <hash name="foo" modulus="10">$foo</hash>
  HashInterpolator(const XML::Element& root);

  //------------------------------------------------------------------------
  // Add a hash generating a property of the given 'name', of hash of
  // string 'pattern' interpolated with existing values and modded with
  // 'modulus'
  void add_hash(const string& name, unsigned int modulus,
                const string& pattern)
  {
    hashes.push_back(Hash(name, modulus, pattern));
  }

  //------------------------------------------------------------------------
  // Augment an existing PropertyList with hashes derived from existing
  // properties
  void augment(PropertyList& pl) const;

  // Comparator for testing
  bool operator==(const HashInterpolator& o) const
  { return hashes == o.hashes; }

  //------------------------------------------------------------------------
  // Get hashes
  list<Hash> get_hashes() const
  { return hashes; }
};

//==========================================================================
// Random string generator
// Uses best available random number source (e.g. /dev/urandom), falls
// back to a Marsaglia MWC in its absence
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
// Hex data dumper
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
// Range set - stores a list of integer ranges - e.g. file fragments, and
// offers various useful operations on them (range.cc)
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

    bool operator<(const Range& b) const
    { return start < b.start; }

    friend ostream& operator<<(ostream& s, const Range& r)
    { return s << r.start << "+" << r.length; }

    // Merge a range with this one
    void merge(const Range& b)
    {
      offset_t e = end();
      start = min(start, b.start);
      length = max(e, b.end()) - start;
    }

    // Remove a range from start
    void remove_from_start(const Range& b)
    {
      offset_t e = end();
      start = max(start, b.end());
      length = e - start;
    }

    // Remove a range from end
    void remove_from_end(const Range& b)
    {
      length = min(end(), b.start) - start;
    }

    // Remove range from middle - returns new range created by split
    Range remove_from_middle(const Range& b)
    {
      offset_t e = end();
      length = b.start - start;
      Range c(b.end(), e - b.end());
      return c;
    }

    // Overlaps in any way including at edge
    bool overlaps_with(const Range& b) const
    {
      offset_t e = end();
      offset_t b_e = b.end();
      return ((start <= b.start && e >= b.start) ||
              (start <= b_e && e >= b_e) ||
              (start >= b.start && e <= b_e));
    }

    // Is contained by a range
    bool contained_by(const Range& b) const
    {
      return start >= b.start && end() <= b.end();
    }

    // Is start overlapped by range?
    bool start_overlapped_by(const Range& b) const
    {
      return start >= b.start && start < b.end();
    }

    // Is end overlapped by range?
    bool end_overlapped_by(const Range& b) const
    {
      offset_t e = end();
      return e > b.start && e <= b.end();
    }
  };

  // Ordered set of ranges
  set<Range> ranges;

  // Expected end offset - provides information for various operations
  // below.  Will be modified by insertions if exceeded - hence 0 is fine
  // if you want just to count the total length seen so far
  offset_t end_offset;

  // Iterator types
  typedef typename set<Range>::iterator iterator;
  iterator begin() { return ranges.begin(); }
  iterator end()   { return ranges.end();   }
  typedef typename set<Range>::const_iterator const_iterator;
  const_iterator begin() const { return ranges.begin(); }
  const_iterator end() const   { return ranges.end();   }
  typedef typename set<Range>::size_type size_type;

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

    Range range(start, length);

    // Find the first range which starts immediately before the new one
    iterator it = ranges.lower_bound(range);
    if ((it == ranges.end() && it != ranges.begin())
        || (it != ranges.end() && it != ranges.begin() && it->start > start))
    {
      --it;
      // Check that the range we're on has some overlap, and if not budge
      // forwards
      if (it->end() < start)
        ++it;
    }

    // Eat and remove any existing ranges that overlap with this one
    while (it != ranges.end() && it->overlaps_with(range))
    {
      // Adjust new one to include it
      range.merge(*it);
      // Remove the existing one and move pointer to next
      iterator eatme = it++;
      ranges.erase(eatme);
    }

    // Insert new range
    ranges.insert(range);

    // Check if end offset exceeded
    if (range.end()>end_offset)
      end_offset = range.end();
  }

  //------------------------------------------------------------------------
  // Insert a range set (inserts every element = set union)
  void insert(const RangeSet& o)
  {
    for(const_iterator p = o.ranges.begin(); p!=o.ranges.end(); ++p)
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

    Range range(start, length);

    // Find the first range which starts immediately before the range
    iterator it = ranges.lower_bound(range);
    if ((it == ranges.end() && it != ranges.begin())
        || (it != ranges.end() && it->start > start))
      --it;

    // Eat any existing ranges that overlap with the range
    while (it != ranges.end() && it->overlaps_with(range))
    {
      // Entirely contained by range
      if (it->contained_by(range))
      {
        // Just eat it
        iterator eatme = it++;
        ranges.erase(eatme);
      }
      // Start overlapped by range
      else if (it->start_overlapped_by(range))
      {
        // Create non-overlapped portion of range
        Range r = *it;
        r.remove_from_start(range);
        // Remove old range
        iterator eatme = it++;
        ranges.erase(eatme);
        // Insert portion
        ranges.insert(r);
      }
      // End overlapped by range
      else if (it->end_overlapped_by(range))
      {
        // Create non-overlapped portion of range
        Range r = *it;
        r.remove_from_end(range);
        // Remove old range
        iterator eatme = it++;
        ranges.erase(eatme);
        // Insert portion
        ranges.insert(r);
      }
      // Entirely contains the range
      else
      {
        // Create non-overlapped portions of range
        Range r = *it;
        Range r2 = r.remove_from_middle(range);
        // Remove old range
        iterator eatme = it++;
        ranges.erase(eatme);
        // Insert portions
        pair<iterator, bool> p = ranges.insert(r);
        ranges.insert(p.first, r2);
      }
    }
  }

  //------------------------------------------------------------------------
  // Remove a range set (removes every element = set difference)
  void remove(const RangeSet& o)
  {
    for(const_iterator p = o.ranges.begin(); p != o.ranges.end(); ++p)
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
    iterator end(inverse.begin());

    // Walk filling in gaps between last end (or 0) and new start
    for(const_iterator p = ranges.begin(); p != ranges.end(); ++p)
    {
      const Range& r = *p;
      if (last)
        end = inverse.ranges.insert(end,
                                    Range(last->end(), r.start-last->end()));
      else if (r.start > 0)
        end = inverse.ranges.insert(end, Range(0, r.start));

      last = &r;
    }

    // Complete with gap (if any) between final end and total length
    if (last)
    {
      if (end_offset > last->end())
        end = inverse.ranges.insert(end,
                                    Range(last->end(), end_offset-last->end()));
    }
    else if (end_offset > 0)
    {
      // Empty set - add whole range
      end = inverse.ranges.insert(end, Range(0, end_offset));
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
    Range range(start, length);

    // Because the set is always optimal, there must be a single range
    // which includes both the start and end
    for(const_iterator p = ranges.begin(); p != ranges.end(); ++p)
    {
      if (range.contained_by(*p))
        return true;
    }

    return false;
  }

  //------------------------------------------------------------------------
  // Get total coverage of the set (sum of all range lengths)
  length_t coverage() const
  {
    length_t sum = 0;

    // Simply add up all the range lengths
    for(const_iterator p = ranges.begin(); p != ranges.end(); ++p)
      sum += p->length;

    return sum;
  }

  //------------------------------------------------------------------------
  // Get number of ranges
  size_type count() const { return ranges.size(); }

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
    const_iterator p = ranges.begin();
    string s;

    if (length > end_offset) length = end_offset;

    for(unsigned int i=0; i<length; i++)
    {
      // Calculate (exactly, avoiding rounding and overflow) start and end
      // of this fraction (end=start of next)
      double this_start = static_cast<double>(end_offset)*i
                          / static_cast<double>(length);
      double this_end   = static_cast<double>(end_offset)*(i+1)
                          / static_cast<double>(length);

      int contained = 0;  // 0 = none, 1 = some, 2 = all

      while (p!=ranges.end())
      {
        const Range& r = *p;

        // Skip to next fraction if not yet overlapping this range
        if (static_cast<double>(r.start) >= this_end) break;

        // Check if we've already left this range
        if (this_start >= static_cast<double>(r.end()))
        {
          ++p;
          continue;
        }

        // So we have an overlap - is it total?
        if (static_cast<double>(r.start) <= this_start
            && static_cast<double>(r.end()) >= this_end)
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

//--------------------------------------------------------------------------
// << operator to write RangeSet to ostream
ostream& operator<<(ostream& s, const UInt64RangeSet& rs);

//==========================================================================
// UUID class
class UUID: public array<byte, 16>
{
public:
  //------------------------------------------------------------------------
  // Constructors
  UUID():
    array<byte, 16>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
  {}
  UUID(byte b0, byte b1, byte b2, byte b3, byte b4, byte b5, byte b6, byte b7,
       byte b8, byte b9, byte ba, byte bb, byte bc, byte bd, byte be, byte bf):
    array<byte, 16>{{b0, b1, b2, b3, b4, b5, b6, b7,
                    b8, b9, ba, bb, bc, bd, be, bf}}
  {}
  // From a string - hex or uuid style
  UUID(string str);

  //------------------------------------------------------------------------
  // Get as string
  string get_str() const;

  //------------------------------------------------------------------------
  // Get as plain hex string
  string get_hex_str() const;

  //------------------------------------------------------------------------
  // Get as base64
  string get_base64_str() const;

  //------------------------------------------------------------------------
  // Set to a random value
  void randomise();

  //------------------------------------------------------------------------
  // Test has a value
  bool operator!() const;
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MISC_H
