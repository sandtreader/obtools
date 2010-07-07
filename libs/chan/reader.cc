//==========================================================================
// ObTools::Chan: reader.cc
//
// Generic protocol/format reader
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"

// Network headers for ntohl
#if defined(__WIN32__)
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#define CHANNEL_BUFFER_SIZE 1024

namespace ObTools { namespace Channel {

//--------------------------------------------------------------------------
// Try to read an exact amount of data from the channel into a binary buffer
// Returns false if channel goes EOF before anything is read
// Buf may be 0 to skip data
// Throws Error on failure, or EOF after a read
bool Reader::try_read(void *buf, size_t count) throw (Error)
{
  char *p = (char *)buf;
  size_t done = 0;

  while (done < count)
  {
    size_t size = basic_read(p, count-done);
    if (size)
    {
      if (p) p += size;
      done += size;
    }
    else if (done)
      throw Error(0, "EOF");
    else
      return false;
  }

  return true;
}

//--------------------------------------------------------------------------
// Read exact amount of data from the channel into a binary buffer
// Buf may be 0 to skip data
// Throws Error on failure
void Reader::read(void *buf, size_t count) throw (Error)
{
  char *p = (char *)buf;
  size_t done = 0;

  while (done < count)
  {
    size_t size = basic_read(p, count-done);
    if (size)
    {
      if (p) p += size;
      done += size;
    }
    else throw Error(0, "EOF");
  }
}

//--------------------------------------------------------------------------
// Read exact amount of data from the channel into a string
// Throws Error on failure
void Reader::read(string& s, size_t count) throw (Error)
{
  char buf[CHANNEL_BUFFER_SIZE+1];
  size_t done = 0;

  while (done < count)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = CHANNEL_BUFFER_SIZE;
    if (count-done < n) n = count-done;

    size_t size = basic_read(buf, n);
    if (size)
    {
      s.append(buf, size);
      done += size;
    }
    else throw Error(0, "EOF in string");
  }
}

//--------------------------------------------------------------------------
// Read a single byte from the channel
// Throws SocketError on failure or EOF
unsigned char Reader::read_byte() throw (Error)
{
  unsigned char n;
  read(&n, 1);
  return n;
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 2-byte integer from the channel
// Throws SocketError on failure or EOF
uint16_t Reader::read_nbo_16() throw (Error)
{
  uint16_t n;
  read(&n, 2);
  return ntohs(n);
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 3-byte integer from the channel
// Throws SocketError on failure or EOF
uint32_t Reader::read_nbo_24() throw (Error)
{
  unsigned char buf[3];
  read(buf, 3);
  return ((uint32_t)buf[0] << 16) + ((uint32_t)buf[1] << 8) + buf[2];
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 4-byte integer from the socket
// Throws SocketError on failure or EOF
uint32_t Reader::read_nbo_32() throw (Error)
{
  uint32_t n;
  read(&n, 4);
  return ntohl(n);
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws Error on non-EOF failure
bool Reader::read_nbo_32(uint32_t& n) throw (Error)
{
  if (!try_read(&n, 4)) return false;
  n=ntohl(n);
  return true;
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 8-byte integer from the socket
// Throws SocketError on failure or EOF
uint64_t Reader::read_nbo_64() throw (Error)
{
  uint64_t n = read_nbo_32();
  return (n<<32) + read_nbo_32();
}

//--------------------------------------------------------------------------
// Read a network byte order 8-byte double from the channel
// Throws Error on failure or EOF
double Reader::read_nbo_double() throw (Error)
{
  if (sizeof(double) != 8) throw Error(8, "Double is not 8 bytes here");
  union { uint64_t n; double f; } u;  // Union type hack
  u.n = read_nbo_64();
  return u.f;
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 2-byte integer from the channel
// Used only for external protocols specified that way
// Throws Error on failure or EOF
uint16_t Reader::read_le_16() throw (Error)
{
  // No easy way to do this with htonl, since if we're big-endian it's 
  // a NOOP anyway - so do it manually
  unsigned char buf[2];
  read(buf, 2);
  return buf[0] + ((uint16_t)buf[1] << 8);
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 3-byte integer from the channel
// Throws Error on failure or EOF
uint32_t Reader::read_le_24() throw (Error)
{
  unsigned char buf[3];
  read(buf, 3);
  return buf[0] + ((uint32_t)buf[1] << 8) + ((uint32_t)buf[2] << 16);
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 4-byte integer from the channel
// Throws Error on failure or EOF
uint32_t Reader::read_le_32() throw (Error)
{
  unsigned char buf[4];
  read(buf, 4);
  return buf[0] + ((uint32_t)buf[1] << 8) + ((uint32_t)buf[2] << 16)
                + ((uint32_t)buf[3] << 24);
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws Error on non-EOF failure
bool Reader::read_le_32(uint32_t& n) throw (Error)
{
  unsigned char buf[4];
  if (!try_read(&buf, 4)) return false;
  n = buf[0] + ((uint32_t)buf[1] << 8) + ((uint32_t)buf[2] << 16)
             + ((uint32_t)buf[3] << 24);
  return true;
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 8-byte integer from the channel
// Throws Error on failure or EOF
uint64_t Reader::read_le_64() throw (Error)
{
  uint64_t n = read_le_32();
  return n + ((uint64_t)read_le_32()<<32);
}

//--------------------------------------------------------------------------
// Read a little-endian 8-byte double from the channel
// Throws Error on failure or EOF
double Reader::read_le_double() throw (Error)
{
  if (sizeof(double) != 8) throw Error(8, "Double is not 8 bytes here");
  union { uint64_t n; double f; } u;  // Union type hack
  u.n = read_le_64();
  return u.f;
}

//--------------------------------------------------------------------------
// Skip N bytes
void Reader::skip(size_t n) throw (Error)
{
  read(0, n);
}

//--------------------------------------------------------------------------
// Skip to given alignment (bytes) from current offset
void Reader::align(size_t n) throw (Error)
{ 
  skip((size_t)(n*((offset+n-1)/n) - offset));  // Bytes to skip
}

}} // namespaces



