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
#include <stdint.h>
#include <iostream>
#include <sstream>

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
  void update(char *buf, unsigned len);

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
//Propertylist class
//Simple sugaring of a string-string map
class PropertyList: public map<string, string>
{
public:
  //------------------------------------------------------------------------
  // Constructor
  PropertyList() {}

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
  // Overridden read-only [] operator
  // Unlike standard [], doesn't create a blank value if it doesn't exist
  string operator[](const string& name) const { return get(name); }

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
  Random() {}

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

public:
  //------------------------------------------------------------------------
  // Constructor
  // width gives number of bytes per line
  // split is interval of spaces between bytes: 0 is no spaces
  Dumper(ostream& _sout, int _width=16, int _split=4): 
    sout(_sout), width(_width), split(_split) {}

  //------------------------------------------------------------------------
  // Dump a block
  void dump(void *block, int length);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MISC_H
















