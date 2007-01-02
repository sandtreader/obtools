//==========================================================================
// ObTools::Text: base64.cc
//
// Base64 encoding/decoding
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

// Encoding list, up to position 61 - 62 and 63 come from configuration
static const char base64_chars[] =
 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

//--------------------------------------------------------------------------
// Encode a binary block
// Split gives length of line to split at - default (76) is to RFC
// Set 0 to suppress split altogether
// line_end is string to split with, and indent for next line
string Base64::encode(const unsigned char *block, size_t length, 
		      int split, const string& line_end)
{
  string base64;

  // Round up length to nearest three bytes
  size_t rlength = 3*((length+2)/3);
  const unsigned char *p = block;
  uint32_t n = 0;
  int count = 1;  
  int chars = 0;

  for(size_t i=0; i<rlength; i++)
  {
    // Accumulate bytes in n, then zeros to padding
    n <<= 8;
    if (i<length)
    {
      n |= *p++;
      count++;
    }

    // Output and clear every 3 bytes
    if ((i%3)==2)
    {
      // Run out up to 4 6-bit indexes
      for(int j=0; j<count; j++)
      {
	// Extract from bit 18 to 23, then shift up for next time
	int in = (n >> 18) & 0x3f;
	n <<= 6;

	// Encode into base64 characters, plus extras
	if (in < 62)
	  base64 += base64_chars[in];
	else if (in == 62)
	  base64 += extra_62;
	else
	  base64 += extra_63;
      }

      // Split if requested
      chars += count;
      if (split && !(chars%split)) base64 += line_end;

      n = 0;
      count = 1;
    }
  }

  // Add padding
  if (pad) for(;length<rlength;length++) base64+=pad;

  return base64;
}

//--------------------------------------------------------------------------
// Encode a 64-bit integer, top byte first (big-endian)
// Will reduce size to 4 bytes if it fits
string Base64::encode(uint64_t n)
{
  unsigned char buf[8];
  int count = 8;
  // Check if it wil fit in 4 bytes (32-bit)
  if (!(n>>32))
  {
    count = 4;
    n <<= 32;
  }
  
  // Extract bytes, top byte first
  for(int i=0; i<count; i++)
  {
    buf[i] = (unsigned char)(n>>56);
    n <<= 8;
  }

  return encode(buf, count, 0);
}

//--------------------------------------------------------------------------
// Encode a binary string - options as encode above
string Base64::encode(const string& binary, int split, 
		      const string& line_end)
{
  return encode((const unsigned char *)binary.c_str(), binary.size(), 
		split, line_end);
}

//--------------------------------------------------------------------------
// Get length of binary block required for decode 
// This is a maximum estimate - real length may be less than this, but
// will never be more
size_t Base64::binary_length(const string& base64)
{
  // Estimate as 3/4 of the total length, rounded up
  // This will be slightly over the mark because of CR-LF
  return (base64.size()*3+3)/4;
}

//--------------------------------------------------------------------------
// Decode a base64 string into a binary block.  
// Returns real length decoded if it fitted, max_length+1 if it didn't
// - but it will never actually write more than max_length bytes
size_t Base64::decode(const string& base64, unsigned char *block, 
		      size_t max_length)
{
  size_t written = 0;
  uint32_t n = 0;
  int i=0;

  for(string::const_iterator p = base64.begin(); p!=base64.end(); ++p)
  {
    char c = *p;
    int cn;

    // Check ranges
    if (c>='A' && c<='Z')
      cn = c-'A';
    else if (c>='a' && c<='z')
      cn = 26+c-'a';
    else if (c>='0' && c<='9')
      cn = 52+c-'0';
    else if (c==extra_62)
      cn = 62;
    else if (c==extra_63)
      cn = 63;
    else if (c==pad)
      break;             // Stop here - we'll fix up the remainder later
    else
      continue;          // Ignore everything else

    // Accumulate into n so first byte ends up at top
    n <<= 6;
    n |= cn;

    // Output every 4 characters
    if (!(++i&3))
    {
      // Turn into 3 bytes, top byte first
      for(int j=0; j<3; j++)
      {
	if (written < max_length)
	  block[written++] = (unsigned char)(n >> 16);
	else
	  return max_length+1;
	n <<=8;
      }

      n=0;
    }
  }

  // Now write out any we have left over - whether or not we saw padding
  if (i&3)
  {
    // Shift up to where it should be
    n <<= 6*(4-(i&3));

    for(int j=0; j<(i&3)-1; j++)
    {
      if (written < max_length)
	block[written++] = (unsigned char)(n >> 16);
      else
	return max_length+1;
      n <<=8;
    }
  }

  return written;
}

//--------------------------------------------------------------------------
// Decode a 64-bit integer, top byte first (big-endian)
// Returns whether successful - if so, sets 'n'
bool Base64::decode(const string& base64, uint64_t& n)
{
  unsigned char buf[8];  
  size_t len = decode(base64, buf, 8);
  if (len > 8) return false;

  // Accumulate in N, top byte first
  n = 0;
  for(size_t i=0; i<len; i++) { n<<=8; n|=buf[i]; }
  return true;
}

//--------------------------------------------------------------------------
// Decode base64 text into the given (binary) string
// Returns whether successful - if so, appends data to binary
// Requires temporary buffer equal to the binary_length() of the string
bool Base64::decode(const string& base64, string& binary)
{
  size_t max_length = binary_length(base64);
  unsigned char *buf = new unsigned char[max_length];

  size_t len = decode(base64, buf, max_length);
  if (len > max_length) return false;

  binary.append((const char *)buf, len);
  return true;
}

}} // namespaces
