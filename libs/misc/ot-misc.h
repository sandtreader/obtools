//==========================================================================
// ObTools::Misc: ot-misc.h
//
// Public definitions for ObTools::Misc
// Miscellaneous useful classes
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_MISC_H
#define __OBTOOLS_MISC_H

#include <string>
#include <map>
#include <list>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include "ot-xml.h"
#include "ot-chan.h"

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

  //------------------------------------------------------------------------
  // Constructor (creates internal combination table, relatively expensive)
  CRC32();

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
  // Copy constructor and assignment operator from map<string, string>
  PropertyList(const map<string, string>& o) 
  { *(map<string, string> *)this = o; }
  PropertyList& operator=(const map<string, string>& o) 
  { *(map<string, string> *)this = o; return *this; }

  //--------------------------------------------------------------------------
  // Add a value
  void add(const string& name, const string& value)
  { insert(make_pair(name,value)); }  //Note, not using []

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
  void add(const string& name, bool value)
  { // Avoid evil recursion due to bool->string implicit cast
    insert(make_pair(name, string(value?"true":"false"))); 
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
};

//------------------------------------------------------------------------
// << operator to write PropertyList to ostream
ostream& operator<<(ostream& s, const PropertyList& pl);

//==========================================================================
//Random string generator
//Uses best available random number source (e.g. /dev/urandom)
class Random
{
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
class RangeSet
{
public:
  // Integer size typedefs 
  typedef uint64_t off_t;
  typedef uint64_t len_t;

  // Internal record of a single range
  struct Range
  {
    off_t start;
    len_t length;
    Range(off_t _start, len_t _length): start(_start), length(_length) {}
    off_t end() const { return start+length; }  // One past the end
  };

  // List of ranges, stored in order
  list<Range> ranges;

  // Expected total length - provides information for various operations 
  // below.  Will be modified by insertions if exceeded - hence 0 is fine
  // if you want just to count the total length seen so far
  len_t total_length;

  // Iterator types
  typedef list<Range>::iterator iterator;
  iterator begin() { return ranges.begin(); }
  iterator end()   { return ranges.end();   }

  //------------------------------------------------------------------------
  // Constructor
  RangeSet(len_t _total_length=0): total_length(_total_length) {}

  //------------------------------------------------------------------------
  // Constructor from string
  RangeSet(const string& text, len_t _total_length=0): 
    total_length(_total_length) { read(text); }

  //------------------------------------------------------------------------
  // Insert a range
  // Optimises against adjacent ranges, overlaps allowed
  void insert(off_t start, len_t length=1);

  //------------------------------------------------------------------------
  // Insert a range set (inserts every element = set union)
  void insert(const RangeSet& o);

  //------------------------------------------------------------------------
  // Set union and addition operator
  RangeSet set_union(const RangeSet& o) const
  { RangeSet r=*this; r.insert(o); return r; }

  RangeSet operator+(const RangeSet& o) const { return set_union(o); }

  //------------------------------------------------------------------------
  // Remove a range
  void remove(off_t start, len_t length);

  //------------------------------------------------------------------------
  // Remove a range set (removes every element = set difference)
  void remove(const RangeSet& o);

  //------------------------------------------------------------------------
  // Set difference, and subtraction operator
  RangeSet difference(const RangeSet& o) const
  { RangeSet r=*this; r.remove(o); return r; }

  RangeSet operator-(const RangeSet& o) { return difference(o); }

  //------------------------------------------------------------------------
  // Return a new set of all the 'holes' in the set, up to the total_length
  RangeSet inverse() const;

  //------------------------------------------------------------------------
  // Return a new set which is the intersection of this set with another
  // and ^ operator
  RangeSet intersection(const RangeSet& o) const;
  RangeSet operator^(const RangeSet& o) const { return intersection(o); }

  //------------------------------------------------------------------------
  // Clear the set
  void clear() { ranges.clear(); }

  //------------------------------------------------------------------------
  // Check if a given range is all present
  bool contains(off_t start, len_t length) const;

  //------------------------------------------------------------------------
  // Get total coverage of the set (sum of all range lengths)
  len_t coverage() const;

  //------------------------------------------------------------------------
  // Get number of ranges
  int count() const { return ranges.size(); }

  //------------------------------------------------------------------------
  // Check if the set is complete up to total_length
  bool is_complete() const { return contains(0, total_length); } 

  //------------------------------------------------------------------------
  // Get percentage coverage
  int percentage_complete() 
  { return total_length?(100*coverage()/total_length):100; }

  //------------------------------------------------------------------------
  // Show the set as a string 'fuel gauge' of the given max string length
  // Each character in the string maps to a fractional part of the range
  // (measured to total_length):
  //   ' ' (space):  No range present in this fraction
  //   '-'        :  Part of range present in this fraction
  //   '='        :  Full range present in this fraction
  // Most simply, this gives a fuel gauge for monotonically increasing
  // ranges, but also allows for insertion at multiple points
  // If length is more than the total length of the range, it is reduced to it
  // (a character can never represent less than 1 unit)
  string gauge(unsigned int length=50) const;

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
  // together.  Also reads total_length attribute of parent if present
  void read_from_xml(const XML::Element& parent, 
		     const string& element_name="range");

  //------------------------------------------------------------------------
  // Convert to XML
  // Adds <range start="x" length="y"/> elements to the given XML element
  // or other element name if provided
  // and adds total_length attribute to parent
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
ostream& operator<<(ostream& s, const RangeSet& rs);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MISC_H
















