//==========================================================================
// ObTools::Text: convert.cc
//
// Useful numeric - string conversion functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <ctype.h>
#include <sstream>
#include <stdio.h>

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
  char buf[21];  // Enough for 64-bit integers, just in case
  snprintf(buf, 21, "%llu", i);
  return string(buf);
}

//--------------------------------------------------------------------------
// String to 64-bit integer (0 default)
uint64_t stoi64(const string& s)
{
  return atoll(s.c_str());
}

//--------------------------------------------------------------------------
// Float to string, with zero padding
string ftos(double f, int width, int prec, bool zero_pad)
{
  // Use sprintf for speed
  char buf[41];  // Enough for %f format
  snprintf(buf, 41, zero_pad?"%#0*.*f":"%#*.*f", width, prec, f);
  return string(buf);
}

//--------------------------------------------------------------------------
// String to float (0.0 default)
double stof(const string& s)
{
  return atof(s.c_str());
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
  int n = 0;
  sscanf(s.c_str(), "%x", &n);
  return n;
}

//--------------------------------------------------------------------------
// 64-bit integer to hex
string i64tox(uint64_t i)
{
  // Use sprintf for speed
  char buf[17];  // Enough for 64-bit integers, just in case
  snprintf(buf, 17, "%llx", i);
  return string(buf);
}

//--------------------------------------------------------------------------
// Hex to 64-bit integer (0 default)
uint64_t xtoi64(const string& s)
{
  uint64_t n = 0;
  sscanf(s.c_str(), "%llx", &n);
  return n;
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
    int n;
    sscanf(buf, "%x", &n);
    data[i] = (unsigned char)n;
  }

  return length;
}

}} // namespaces
