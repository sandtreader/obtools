//==========================================================================
// ObTools::Misc: ot-misc.h
//
// Public definitions for ObTools::Text 
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
  // Get a value, with default
  string get(const string& name, const string& def="") const
  {
    const_iterator p = find(name);
    if (p!=end()) return p->second;
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
// << operator to write Cloud to ostream
ostream& operator<<(ostream& s, const PropertyList& pl);
 
//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_MISC_H
















