//==========================================================================
// ObTools::Chan: writer.cc
//
// Generic protocol/format writer
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"

// Network headers for htonl
#if defined(__WIN32__)
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

namespace ObTools { namespace Channel {

//--------------------------------------------------------------------------
// Write a string to the channel
// Throws Error on failure
void Writer::write(const string& s) throw (Error)
{
  basic_write(s.c_str(), s.size());
}

//--------------------------------------------------------------------------
// Write the given C string to the channel
// Throws Error on failure
void Writer::write(const char *p) throw(Error)
{
  basic_write(p, strlen(p));
}

//--------------------------------------------------------------------------
// Write a single byte to the channel
// Throws Error on failure
void Writer::write_byte(unsigned char b) throw (Error)
{
  basic_write(&b, 1);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 2-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_16(uint16_t i) throw (Error)
{
  uint16_t n = htons(i);
  basic_write(&n, 2);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 3-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_24(uint32_t i) throw (Error)
{
  unsigned char buf[3];
  buf[0] = (unsigned char)(i>>16);
  buf[1] = (unsigned char)(i>>8);
  buf[2] = (unsigned char)(i);
  basic_write(buf, 3);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 4-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_32(uint32_t i) throw (Error)
{
  uint32_t n = htonl(i);
  basic_write(&n, 4);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 8-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_64(uint64_t i) throw (Error)
{
  write_nbo_32((uint32_t)(i >> 32));
  write_nbo_32((uint32_t)(i & 0xFFFFFFFF));
}

//--------------------------------------------------------------------------
// Skip N bytes, writing zero
void Writer::skip(size_t n) throw (Error)
{
  static char buf[256] = { 0 };
  while (n)
  {
    size_t i = n;
    if (i > 256) i=256;
    basic_write(buf, i); 
    n-=i;
  }
}

//--------------------------------------------------------------------------
// Pad to given alignment (bytes) from current offset
void Writer::align(size_t n) throw (Error)
{ 
  skip((int)(n*((offset+n-1)/n) - offset));  // Bytes to pad
}

}} // namespaces



