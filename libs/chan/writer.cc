//==========================================================================
// ObTools::Chan: writer.cc
//
// Generic protocol/format writer
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
// Pad to given alignment (bytes) from current offset
void Writer::align(int n)
{ 
  char c = 0;
  int i = n*((offset+n-1)/n) - offset;  // Bytes to pad
  while (i--) basic_write(&c, 1); 
}

}} // namespaces



