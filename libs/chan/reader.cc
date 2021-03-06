//==========================================================================
// ObTools::Chan: reader.cc
//
// Generic protocol/format reader
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include "math.h"

// Network headers for ntohl
#if defined(PLATFORM_WINDOWS)
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
bool Reader::try_read(void *buf, size_t count)
{
  char *p = static_cast<char *>(buf);
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
void Reader::read(void *buf, size_t count)
{
  if (!try_read(buf, count))
    throw Error(0, "EOF");
}

//--------------------------------------------------------------------------
// Try to read an exact amount of data from the channel into a string
// Returns false if channel goes EOF before anything is read
// Buf may be 0 to skip data
// Throws Error on failure, or EOF after a read
bool Reader::try_read(string& s, size_t count)
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
    else if (done)
      throw Error(0, "EOF in string");
    else
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------
// Read exact amount of data from the channel into a string
// Throws Error on failure
void Reader::read(string& s, size_t count)
{
  if (!try_read(s, count))
    throw Error(0, "EOF in string");
}

//--------------------------------------------------------------------------
// Read data into buf until EOF or limit encountered
void Reader::read_to_eof(vector<unsigned char>& buffer, size_t limit)
{
  char buf[CHANNEL_BUFFER_SIZE+1];
  size_t done = 0;

  while (done < limit)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = CHANNEL_BUFFER_SIZE;
    if (limit-done < n) n = limit-done;

    size_t size = basic_read(buf, n);
    if (size)
    {
      buffer.insert(buffer.end(), &buf[0], &buf[size]);
      done += size;
    }
    else
      return;
  }
}

//--------------------------------------------------------------------------
// Read data into buf until EOF or limit encountered
void Reader::read_to_eof(unsigned char *buffer, size_t limit)
{
  char buf[CHANNEL_BUFFER_SIZE+1];
  size_t done = 0;

  while (done < limit)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = CHANNEL_BUFFER_SIZE;
    if (limit-done < n) n = limit-done;

    size_t size = basic_read(buf, n);
    if (size)
    {
      copy(&buf[0], &buf[size], buffer + done);
      done += size;
    }
    else
      return;
  }
}

//--------------------------------------------------------------------------
// Read data into buf until EOF
void Reader::read_to_eof(vector<unsigned char>& buffer)
{
  char buf[CHANNEL_BUFFER_SIZE+1];

  while (true)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = CHANNEL_BUFFER_SIZE;
    size_t size = basic_read(buf, n);
    if (size)
    {
      buffer.insert(buffer.end(), &buf[0], &buf[size]);
    }
    if (size != n)
      return;
  }
}

//--------------------------------------------------------------------------
// Read data into string until EOF or limit encountered
void Reader::read_to_eof(string& s, size_t limit)
{
  char buf[CHANNEL_BUFFER_SIZE+1];
  size_t done = 0;

  while (done < limit)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = CHANNEL_BUFFER_SIZE;
    if (limit-done < n) n = limit-done;

    size_t size = basic_read(buf, n);
    if (size)
    {
      s.append(&buf[0], size);
      done += size;
    }
    else
      return;
  }
}

//--------------------------------------------------------------------------
// Read data into string until EOF
void Reader::read_to_eof(string& s)
{
  char buf[CHANNEL_BUFFER_SIZE+1];

  while (true)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = CHANNEL_BUFFER_SIZE;
    size_t size = basic_read(buf, n);
    if (size)
    {
      s.append(&buf[0], size);
    }
    if (size != n)
      return;
  }
}

//--------------------------------------------------------------------------
// Try to read a single byte from the channel
// Throws Error on failure
bool Reader::try_read_byte(unsigned char& b)
{
  return try_read(&b, 1);
}

//--------------------------------------------------------------------------
// Read a single byte from the channel
// Throws SocketError on failure or EOF
unsigned char Reader::read_byte()
{
  unsigned char n;
  read(&n, 1);
  return n;
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 2-byte integer from the channel
// Throws SocketError on failure or EOF
uint16_t Reader::read_nbo_16()
{
  uint16_t n;
  read(&n, 2);
  return ntohs(n);
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 3-byte integer from the channel
// Throws SocketError on failure or EOF
uint32_t Reader::read_nbo_24()
{
  unsigned char buf[3];
  read(buf, 3);
  return (static_cast<uint32_t>(buf[0]) << 16)
         + (static_cast<uint32_t>(buf[1]) << 8) + buf[2];
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 4-byte integer from the socket
// Throws SocketError on failure or EOF
uint32_t Reader::read_nbo_32()
{
  uint32_t n;
  read(&n, 4);
  return ntohl(n);
}

//--------------------------------------------------------------------------
// Try to read a network byte order (MSB-first) 4-byte integer
// Throws Error on failure
bool Reader::try_read_nbo_32(uint32_t& n)
{
  if (!try_read(&n, 4))
    return false;

  n = ntohl(n);
  return true;
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws Error on non-EOF failure
bool Reader::read_nbo_32(uint32_t& n)
{
  if (!try_read(&n, 4)) return false;
  n=ntohl(n);
  return true;
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 8-byte integer from the socket
// Throws SocketError on failure or EOF
uint64_t Reader::read_nbo_64()
{
  uint64_t n = read_nbo_32();
  return (n<<32) + read_nbo_32();
}

//--------------------------------------------------------------------------
// Read a network byte order 8-byte double from the channel
// Throws Error on failure or EOF
double Reader::read_nbo_double()
{
  if (sizeof(double) != 8) throw Error(8, "Double is not 8 bytes here");
  union { uint64_t n; double f; } u;  // Union type hack
  u.n = read_nbo_64();
  return u.f;
}

//--------------------------------------------------------------------------
// Read a fixed-point number from the channel
double Reader::read_nbo_fixed_point(int before_bits, int after_bits)
{
  int bits = before_bits + after_bits;
  if (bits % 8 || bits > 64)
    throw Error(9, "Total number of bits must be order of 8 "
                   "and no greater than 64");
  uint64_t n(0);
  while (bits)
  {
    n = n << 8;
    n += read_byte();
    bits -= 8;
  }
  return n / pow(2, after_bits);
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 2-byte integer from the channel
// Used only for external protocols specified that way
// Throws Error on failure or EOF
uint16_t Reader::read_le_16()
{
  // No easy way to do this with htonl, since if we're big-endian it's
  // a NOOP anyway - so do it manually
  unsigned char buf[2];
  read(buf, 2);
  return buf[0] + (static_cast<uint16_t>(buf[1]) << 8);
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 3-byte integer from the channel
// Throws Error on failure or EOF
uint32_t Reader::read_le_24()
{
  unsigned char buf[3];
  read(buf, 3);
  return buf[0] + (static_cast<uint32_t>(buf[1]) << 8)
                + (static_cast<uint32_t>(buf[2]) << 16);
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 4-byte integer from the channel
// Throws Error on failure or EOF
uint32_t Reader::read_le_32()
{
  unsigned char buf[4];
  read(buf, 4);
  return buf[0] + (static_cast<uint32_t>(buf[1]) << 8)
                + (static_cast<uint32_t>(buf[2]) << 16)
                + (static_cast<uint32_t>(buf[3]) << 24);
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws Error on non-EOF failure
bool Reader::read_le_32(uint32_t& n)
{
  unsigned char buf[4];
  if (!try_read(&buf, 4)) return false;
  n = buf[0] + (static_cast<uint32_t>(buf[1]) << 8)
             + (static_cast<uint32_t>(buf[2]) << 16)
             + (static_cast<uint32_t>(buf[3]) << 24);
  return true;
}

//--------------------------------------------------------------------------
// Read a little-endian (LSB-first) 8-byte integer from the channel
// Throws Error on failure or EOF
uint64_t Reader::read_le_64()
{
  uint64_t n = read_le_32();
  return n + (static_cast<uint64_t>(read_le_32())<<32);
}

//--------------------------------------------------------------------------
// Read a little-endian 8-byte double from the channel
// Throws Error on failure or EOF
double Reader::read_le_double()
{
  if (sizeof(double) != 8) throw Error(8, "Double is not 8 bytes here");
  union { uint64_t n; double f; } u;  // Union type hack
  u.n = read_le_64();
  return u.f;
}

//--------------------------------------------------------------------------
// Skip to EOF
void Reader::skip_to_eof()
{
  while (try_read(0, CHANNEL_BUFFER_SIZE));
}

//--------------------------------------------------------------------------
// Skip N bytes
void Reader::skip(size_t n)
{
  read(0, n);
}

//--------------------------------------------------------------------------
// Skip to given alignment (bytes) from current offset
void Reader::align(size_t n)
{
  skip(static_cast<size_t>(n*((offset+n-1)/n) - offset));  // Bytes to skip
}

}} // namespaces
