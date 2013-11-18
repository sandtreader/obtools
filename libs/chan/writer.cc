//==========================================================================
// ObTools::Chan: writer.cc
//
// Generic protocol/format writer
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include "math.h"

// Network headers for ntohl
#if defined(__WIN32__)
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

namespace ObTools { namespace Channel {

//--------------------------------------------------------------------------
// Write a string to the channel
// Throws Error on failure
void Writer::write(const string& s)
{
  basic_write(s.c_str(), s.size());
}

//--------------------------------------------------------------------------
// Write the given C string to the channel
// Throws Error on failure
void Writer::write(const char *p)
{
  basic_write(p, strlen(p));
}

//--------------------------------------------------------------------------
// Write a single byte to the channel
// Throws Error on failure
void Writer::write_byte(unsigned char b)
{
  basic_write(&b, 1);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 2-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_16(uint16_t i)
{
  uint16_t n = htons(i);
  basic_write(&n, 2);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 3-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_24(uint32_t i)
{
  unsigned char buf[3];
  buf[0] = static_cast<unsigned char>(i>>16);
  buf[1] = static_cast<unsigned char>(i>>8);
  buf[2] = static_cast<unsigned char>(i);
  basic_write(buf, 3);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 4-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_32(uint32_t i)
{
  uint32_t n = htonl(i);
  basic_write(&n, 4);
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 8-byte integer to the channel
// Throws Error on failure
void Writer::write_nbo_64(uint64_t i)
{
  write_nbo_32(static_cast<uint32_t>(i >> 32));
  write_nbo_32(static_cast<uint32_t>(i & 0xFFFFFFFF));
}

//--------------------------------------------------------------------------
// Write a network byte order 8-byte double to the channel
// Throws Error on failure or EOF
void Writer::write_nbo_double(double f)
{
  if (sizeof(double) != 8) throw Error(8, "Double is not 8 bytes here");
  union { uint64_t n; double f; } u;  // Union type hack
  u.f = f;
  write_nbo_64(u.n);
}

//--------------------------------------------------------------------------
// Write a network byte order fixed-point double
void Writer::write_nbo_fixed_point(double f, int before_bits, int after_bits)
{
  int bits = before_bits + after_bits;
  if (bits % 8 || bits > 64)
    throw Error(9, "Total number of bits must be order of 8 "
                   "and no greater than 64");
  uint64_t n = f * pow(2, after_bits);
  while (bits)
  {
    bits -= 8;
    write_byte(n >> bits);
  }
}

//--------------------------------------------------------------------------
// Write little-endian (LSB-first) 2-byte integer to the channel
// Throws Error on failure
void Writer::write_le_16(uint16_t i)
{
  // No easy way to do this with htonl, since if we're big-endian it's 
  // a NOOP anyway - so do it manually
  unsigned char buf[2];
  buf[0] = static_cast<unsigned char>(i);
  buf[1] = static_cast<unsigned char>(i>>8);
  basic_write(buf, 2);
}

//--------------------------------------------------------------------------
// Write a little-endian (LSB-first) 3-byte integer to the channel
// Throws Error on failure
void Writer::write_le_24(uint32_t i)
{
  unsigned char buf[3];
  buf[0] = static_cast<unsigned char>(i);
  buf[1] = static_cast<unsigned char>(i>>8);
  buf[2] = static_cast<unsigned char>(i>>16);
  basic_write(buf, 3);
}

//--------------------------------------------------------------------------
// Write a little-endian (LSB-first) 4-byte integer to the channel
// Throws Error on failure
void Writer::write_le_32(uint32_t i)
{
  unsigned char buf[4];
  buf[0] = static_cast<unsigned char>(i);
  buf[1] = static_cast<unsigned char>(i>>8);
  buf[2] = static_cast<unsigned char>(i>>16);
  buf[3] = static_cast<unsigned char>(i>>24);
  basic_write(buf, 4);
}

//--------------------------------------------------------------------------
// Write a little-endian (LSB-first) 8-byte integer to the channel
// Throws Error on failure
void Writer::write_le_64(uint64_t i)
{
  write_nbo_32(static_cast<uint32_t>(i & 0xFFFFFFFF));
  write_nbo_32(static_cast<uint32_t>(i >> 32));
}

//--------------------------------------------------------------------------
// Write a little-endian 8-byte double to the channel
// Throws Error on failure or EOF
void Writer::write_le_double(double f)
{
  if (sizeof(double) != 8) throw Error(8, "Double is not 8 bytes here");
  union { uint64_t n; double f; } u;  // Union type hack
  u.f = f;
  write_le_64(u.n);
}

//--------------------------------------------------------------------------
// Skip N bytes, writing zero
void Writer::skip(size_t n)
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
void Writer::align(size_t n)
{ 
  skip(static_cast<int>(n*((offset+n-1)/n) - offset));  // Bytes to pad
}

}} // namespaces



