//==========================================================================
// ObTools::Channel: ot-chan.h
//
// Public definitions for ObTools::Channel
// Structured Protocol/Format reading and writing
// 
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_CHAN_H
#define __OBTOOLS_CHAN_H

#include "ot-net.h"

namespace ObTools { namespace Channel { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Channel exceptions
class Error 
{
public:
  int error;  // errno value, or 0
  string text; 
  Error(int e=0): error(e), text("") {}
  Error(int e, const string& t): error(e), text(t) {}
};

//------------------------------------------------------------------------
// << operator to write Error to ostream
// e.g. cout << e;
ostream& operator<<(ostream& s, const Error& e);

//==========================================================================
// Abstract Channel Reader
class Reader
{
protected:
  size_t offset;

public:
  //--------------------------------------------------------------------------
  // Basic constructor, destructor
  Reader(): offset(0) {}
  virtual ~Reader() {}

  //--------------------------------------------------------------------------
  // Read as much data as is available, up to 'count' bytes
  // Returns amount read, also adjusts offset
  // Throws Error on failure (not EOF)
  virtual size_t basic_read(void *buf, size_t count) throw (Error) = 0;

  //--------------------------------------------------------------------------
  // Try to read an exact amount of data from the channel into a binary buffer
  // Returns false if channel goes EOF before anything is read
  // Throws Error on failure, or EOF after a read
  bool try_read(void *buf, size_t count) throw (Error);

  //--------------------------------------------------------------------------
  // Read exact amount of data from the channel into a binary buffer
  // Throws Error on failure
  void read(void *buf, size_t count) throw (Error);

  //--------------------------------------------------------------------------
  // Read exact amount of data from the channel into a string
  // Throws Error on failure
  void read(string& s, size_t count) throw (Error);

  //--------------------------------------------------------------------------
  // Read a single byte from the channel
  // Throws SocketError on failure or EOF
  unsigned char read_byte() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 2-byte integer from the channel
  // Throws SocketError on failure or EOF
  uint16_t read_nbo_16() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 3-byte integer from the channel
  // Throws SocketError on failure or EOF
  uint32_t read_nbo_24() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 4-byte integer from the channel
  // Throws SocketError on failure or EOF
  uint32_t read_nbo_32() throw (Error);

  //--------------------------------------------------------------------------
  // Ditto, but allowing the possibility of failure at EOF
  // Throws Error on non-EOF failure
  bool read_nbo_32(uint32_t& n) throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 8-byte integer from the channel
  // Throws SocketError on failure or EOF
  uint64_t read_nbo_64() throw (Error);

  //--------------------------------------------------------------------------
  // Get current offset
  size_t get_offset() { return offset; }

  //--------------------------------------------------------------------------
  // Skip to given alignment (bytes) from current offset
  void align(int n);
};

//==========================================================================
// Abstract Channel Writer
class Writer
{
protected:
  size_t offset;

public:
  //--------------------------------------------------------------------------
  // Basic constructor, destructor
  Writer(): offset(0) {}
  virtual ~Writer() {}

  //--------------------------------------------------------------------------
  // Write exact amount of data to the channel from a binary buffer
  // Also adjusts offset
  // Throws Error on failure
  virtual void basic_write(const void *buf, size_t count) throw (Error) = 0;

  //--------------------------------------------------------------------------
  // Write a string to the channel
  // Throws Error on failure
  void write(const string& s) throw (Error);

  //--------------------------------------------------------------------------
  // Write the given C string to the channel
  // Throws Error on failure
  void write(const char *p) throw(Error);

  //--------------------------------------------------------------------------
  // Write a single byte to the channel
  // Throws Error on failure
  void write_byte(unsigned char b) throw (Error);

  //--------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 2-byte integer to the channel
  // Throws Error on failure
  void write_nbo_16(uint16_t i) throw (Error);

  //--------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 3-byte integer to the channel
  // Throws Error on failure
  void write_nbo_24(uint32_t i) throw (Error);

  //--------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 4-byte integer to the channel
  // Throws Error on failure
  void write_nbo_32(uint32_t i) throw (Error);

  //--------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 8-byte integer to the channel
  // Throws Error on failure
  void write_nbo_64(uint64_t i) throw (Error);

  //--------------------------------------------------------------------------
  // Get current offset
  size_t get_offset() { return offset; }

  //--------------------------------------------------------------------------
  // Pad to given alignment (bytes) from given current offset
  void align(int n);
};

//==========================================================================
// TCP Socket Reader
class TCPSocketReader: public Reader
{
private:
  Net::TCPSocket& s;

public:
  // Constructor
  TCPSocketReader(Net::TCPSocket& _s): s(_s) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count) throw (Error);
};

//==========================================================================
// TCP Socket Writer
class TCPSocketWriter: public Writer
{
private:
  Net::TCPSocket& s;

public:
  // Constructor
  TCPSocketWriter(Net::TCPSocket& _s): s(_s) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count) throw (Error);
};

//==========================================================================
// Binary block reader
// e.g. Wrap around a UDP datagram that has already been read
class BlockReader: public Reader
{
private:
  const char *data;
  size_t length;

public:
  // Constructor
  BlockReader(const char *_data, size_t _length): 
    data(_data), length(_length) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count) throw (Error);
};

//==========================================================================
// Binary block writer
class BlockWriter: public Writer
{
private:
  char *data;
  size_t length;

public:
  // Constructor.  _length is max length - fails after this
  BlockWriter(char *_data, size_t _length): 
    data(_data), length(_length) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count) throw (Error);

  // Get length remaining in block
  size_t get_remaining() { return length-offset; }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CHAN_H


