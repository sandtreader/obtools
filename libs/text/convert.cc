//==========================================================================
// ObTools::Text: convert.cc
//
// Useful numeric - string conversion functions
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <stdio.h>

// MinGW specials
#ifdef MINGW
#define FORMAT_UNSIGNED_64 "%I64u"
#define FORMAT_HEX_64 "%I64x"
#else
#define FORMAT_UNSIGNED_64 "%llu"
#define FORMAT_HEX_64 "%llx"
#endif

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Integer to string
string itos(int i)
{
  // Use sprintf for speed
  char buf[21];  // Enough for 64-bit integers, just in case
  snprintf(buf, 21, "%d", i);
  return string(buf);
}

//--------------------------------------------------------------------------
// String to integer (0 default)
int stoi(const string& s)
{
  return atoi(s.c_str());
}

//--------------------------------------------------------------------------
// 64-bit integer to string
string i64tos(uint64_t i)
{
  // Use sprintf for speed
  char buf[21];  // Enough for 64-bits
  snprintf(buf, 21, FORMAT_UNSIGNED_64, static_cast<unsigned long long>(i));
  return string(buf);
}

//--------------------------------------------------------------------------
// String to 64-bit integer (0 default)
uint64_t stoi64(const string& s)
{
  // Note:  We can't use atoll here because it is signed and barfs on
  // anything over 2^63-1
  unsigned long long n = 0;
  sscanf(s.c_str(), FORMAT_UNSIGNED_64, &n);
  return static_cast<uint64_t>(n);
}

//--------------------------------------------------------------------------
// Integer representing a fixed point to string
string ifixtos(int i, int decimal_places)
{
  string s = itos(i);

  if (!decimal_places)
    return s;

  if (decimal_places < 0)
  {
    // Pad end with zeroes
    for (;decimal_places < 0; ++decimal_places) s+= '0';
    return s;
  }

  int negmod = (i < 0) ? 1 : 0; // Take account of minus sign if present
  // Pad front with zeroes if string is shorter than desired dp
  while (s.size() < static_cast<unsigned int>(decimal_places) + 1 + negmod)
    s.insert(negmod, "0");
  // Insert decimal point
  s.insert(s.size() - decimal_places, ".");
  return s;
}

//--------------------------------------------------------------------------
// String to integer representing a fixed point (assumes correctness)
int stoifix(const string& s, int decimal_places)
{
  if (decimal_places <= 0)
    return Text::stoi(s.substr(0, s.size() + decimal_places));
  string sfix = s;
  sfix.erase(s.size() - decimal_places - 1, 1);
  return Text::stoi(sfix);
}

//--------------------------------------------------------------------------
// Float to string, with zero padding
string ftos(double f, int width, int prec, bool zero_pad)
{
  ostringstream oss;
  if (width) oss << setw(width);
  if (prec) oss << setprecision(prec)
                << setiosflags(ios::fixed | ios::showpoint);
  if (zero_pad) oss << setfill('0');
  oss << f;
  return oss.str();
}

//--------------------------------------------------------------------------
// String to float (0.0 default)
double stof(const string& s)
{
  return atof(s.c_str());
}

//--------------------------------------------------------------------------
// String to boolean (false default)
// Accepts [TtYy1]* as true
bool stob(const string& s)
{
  if (s.empty()) return false;
  switch(s[0])
  {
    case 'T': case 't':
    case 'Y': case 'y':
    case '1':
      return true;

    default:
      return false;
  }
}

//--------------------------------------------------------------------------
// Integer to hex
string itox(unsigned int i)
{
  // Use sprintf for speed
  char buf[17];  // Enough for 64-bit integers, just in case
  snprintf(buf, 17, "%x", i);
  return string(buf);
}

//--------------------------------------------------------------------------
// Hex to integer (0 default)
unsigned int xtoi(const string& s)
{
  unsigned int n = 0;
  sscanf(s.c_str(), "%x", &n);
  return n;
}

//--------------------------------------------------------------------------
// 64-bit integer to hex
string i64tox(uint64_t i)
{
  // Use sprintf for speed
  char buf[17];  // Enough for 64-bit integers, just in case
  snprintf(buf, 17, FORMAT_HEX_64, static_cast<unsigned long long>(i));
  return string(buf);
}

//--------------------------------------------------------------------------
// Hex to 64-bit integer (0 default)
uint64_t xtoi64(const string& s)
{
  unsigned long long n = 0;
  sscanf(s.c_str(), FORMAT_HEX_64, &n);
  return static_cast<uint64_t>(n);
}

//--------------------------------------------------------------------------
// Binary to hex (simple, use Misc::Dumper for long blocks)
string btox(const unsigned char *data, unsigned int length)
{
  string s;
  for(;length>0; length--,data++)
  {
    char buf[3];
    snprintf(buf, 3, "%02x", *data);
    s += buf;
  }

  return s;
}

//--------------------------------------------------------------------------
// Binary to hex (simple, use Misc::Dumper for long blocks)
string btox(const string& data)
{
  string s;
  for(string::const_iterator p=data.begin();p!=data.end();++p)
  {
    char buf[3];
    snprintf(buf, 3, "%02x", static_cast<unsigned char>(*p));
    s += buf;
  }

  return s;
}

//--------------------------------------------------------------------------
// Binary to hex (simple, use Misc::Dumper for long blocks)
string btox(const vector<uint8_t>& data)
{
  ostringstream os;
  os << hex << setfill('0');
  for (vector<uint8_t>::const_iterator p = data.begin(); p != data.end(); ++p)
  {
    os << setw(2) << static_cast<int>(*p);
  }

  return os.str();
}

//--------------------------------------------------------------------------
// Hex to binary
// Reads up to max_length bytes into data, returns number actually read
unsigned int xtob(const string& hex, unsigned char *data,
                  unsigned int max_length)
{
  unsigned int length = hex.size()/2;
  if (length > max_length) length = max_length;

  const char *s = hex.c_str();
  for(unsigned int i=0; i<length; i++)
  {
    char buf[3];
    memcpy(buf, s+2*i, 2);
    buf[2] = 0;
    unsigned int n;
    sscanf(buf, "%x", &n);
    data[i] = static_cast<unsigned char>(n);
  }

  return length;
}

}} // namespaces
