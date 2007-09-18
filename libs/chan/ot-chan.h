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
#include <iostream>

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
  uint64_t offset;

public:
  //--------------------------------------------------------------------------
  // Basic constructor, destructor
  Reader(): offset(0) {}
  virtual ~Reader() {}

  //--------------------------------------------------------------------------
  // Read as much data as is available, up to 'count' bytes
  // Returns amount read, also adjusts offset
  // If 'buf' is 0, just skip as much data as possible
  // Throws Error on failure (not EOF)
  virtual size_t basic_read(void *buf, size_t count) throw (Error) = 0;

  //--------------------------------------------------------------------------
  // Try to read an exact amount of data from the channel into a binary buffer
  // Returns false if channel goes EOF before anything is read
  // Buf may be 0 to skip data
  // Throws Error on failure, or EOF after a read
  bool try_read(void *buf, size_t count) throw (Error);

  //--------------------------------------------------------------------------
  // Read exact amount of data from the channel into a binary buffer
  // Buf may be 0 to skip data
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
  uint64_t get_offset() { return offset; }

  //--------------------------------------------------------------------------
  // Skip N bytes
  virtual void skip(size_t n) throw (Error);

  //--------------------------------------------------------------------------
  // Skip to given alignment (bytes) from current offset
  void align(size_t n) throw (Error);

  //--------------------------------------------------------------------------
  // Whether rewindable - not by default
  virtual bool rewindable() { return false; }

  //--------------------------------------------------------------------------
  // Rewind N bytes - can't do it by default
  virtual void rewind(size_t) throw (Error) { throw Error(2, "Can't rewind"); }

  //--------------------------------------------------------------------------
  // Rewind to beginning
  void rewind() throw (Error) { rewind(offset); }
};

//==========================================================================
// Abstract Channel Writer
class Writer
{
protected:
  uint64_t offset;

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
  // Write exact amount of data to the channel from a binary buffer
  // Aliased to match Reader::read()
  void write(const void *buf, size_t count) throw (Error)
  { basic_write(buf, count); }

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
  uint64_t get_offset() { return offset; }

  //--------------------------------------------------------------------------
  // Skip N bytes, writing zero
  virtual void skip(size_t n) throw (Error);

  //--------------------------------------------------------------------------
  // Pad to given alignment (bytes) from given current offset
  void align(size_t n) throw (Error);

  //--------------------------------------------------------------------------
  // Whether rewindable - not by default
  virtual bool rewindable() { return false; }

  //--------------------------------------------------------------------------
  // Rewind N bytes - can't do it by default
  virtual void rewind(size_t) throw (Error) { throw Error(2, "Can't rewind"); }

  //--------------------------------------------------------------------------
  // Rewind to beginning
  void rewind() throw (Error) { rewind(offset); }
};

//==========================================================================
// Stream Reader
class StreamReader: public Reader
{
private:
  istream& sin;

public:
  // Constructor
  StreamReader(istream& _s): sin(_s) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count) throw (Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Error);
  void rewind() { Reader::rewind(); }
};

//==========================================================================
// Stream Writer
class StreamWriter: public Writer
{
private:
  ostream& sout;

public:
  // Constructor
  StreamWriter(ostream& _s): sout(_s) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count) throw (Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Error);
  void rewind() { Writer::rewind(); }
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
  const unsigned char *data;
  size_t length;

public:
  // Constructor
  BlockReader(const unsigned char *_data, size_t _length): 
    data(_data), length(_length) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count) throw (Error);
  virtual void skip(size_t n) throw (Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Error);
  void rewind() { Reader::rewind(); }
};

//==========================================================================
// Binary block writer
class BlockWriter: public Writer
{
private:
  unsigned char *data;
  size_t length;

public:
  // Constructor.  _length is max length - fails after this
  BlockWriter(unsigned char *_data, size_t _length): 
    data(_data), length(_length) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count) throw (Error);
  virtual void skip(size_t n) throw (Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Error);
  void rewind() { Writer::rewind(); }

  // Get length remaining in block
  size_t get_remaining() { return length-offset; }
};

//==========================================================================
// String reader
class StringReader: public Reader
{
private:
  const string& data;

public:
  // Constructor
  StringReader(const string& _data):  data(_data) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count) throw (Error);
  virtual void skip(size_t n) throw (Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Error);
  void rewind() { Reader::rewind(); }
};

//==========================================================================
// String writer
class StringWriter: public Writer
{
private:
  string& data;

public:
  // Constructor  
  StringWriter(string& _data): data(_data) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count) throw (Error);
  virtual void skip(size_t n) throw (Error);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n) throw (Error);
  void rewind() { Writer::rewind(); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CHAN_H



