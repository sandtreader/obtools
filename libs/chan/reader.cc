//==========================================================================
// ObTools::Chan: reader.cc
//
// Generic protocol/format reader
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
// Read exact amount of data from the channel into a binary buffer
// Returns whether successful (channel not closed)
// Throws Error on failure
bool Reader::read(void *buf, size_t count) throw (Error)
{
  char *p = (char *)buf;
  size_t done = 0;

  while (done < count)
  {
    size_t size = basic_read(p, count-done);
    if (size)
    {
      p += size;
      done += size;
    }
    else return false;
  }
  
  return true;
}

//--------------------------------------------------------------------------
// Read exact amount of data from the channel into a string
// Whether successful - all data was read before channel closed
// Throws Error on failure
bool Reader::read(string& s, size_t count) throw (Error)
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
    else return false;
  }
  
  return true;
}

//--------------------------------------------------------------------------
// Read a single byte from the channel
// Throws SocketError on failure or EOF
unsigned char Reader::read_byte() throw (Error)
{
  unsigned char n;
  if (!read(&n, 1)) throw Error(0, "EOF");
  return n;
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 2-byte integer from the channel
// Throws SocketError on failure or EOF
uint16_t Reader::read_nbo_16() throw (Error)
{
  uint16_t n;
  if (!read(&n, 2)) throw Error(0, "EOF");
  return ntohs(n);
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 3-byte integer from the channel
// Throws SocketError on failure or EOF
uint32_t Reader::read_nbo_24() throw (Error)
{
  unsigned char buf[3];
  if (!read(buf, 3)) throw Error(0, "EOF");
  return ((uint32_t)buf[0] << 16) + ((uint32_t)buf[1] << 8) + buf[2];
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 4-byte integer from the socket
// Throws SocketError on failure or EOF
uint32_t Reader::read_nbo_32() throw (Error)
{
  uint32_t n;
  if (!read(&n, 4)) throw Error(0, "EOF");
  return ntohl(n);
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws Error on non-EOF failure
bool Reader::read_nbo_32(uint32_t& n) throw (Error)
{
  if (!read(&n, 4)) return false;
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

}} // namespaces



