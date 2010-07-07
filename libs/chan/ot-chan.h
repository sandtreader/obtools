//==========================================================================
// ObTools::Channel: ot-chan.h
//
// Public definitions for ObTools::Channel
// Structured Protocol/Format reading and writing
// 
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  // Throws Error on failure or EOF
  unsigned char read_byte() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 2-byte integer from the channel
  // Throws Error on failure or EOF
  uint16_t read_nbo_16() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 3-byte integer from the channel
  // Throws Error on failure or EOF
  uint32_t read_nbo_24() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 4-byte integer from the channel
  // Throws Error on failure or EOF
  uint32_t read_nbo_32() throw (Error);

  //--------------------------------------------------------------------------
  // Ditto, but allowing the possibility of failure at EOF
  // Throws Error on non-EOF failure
  bool read_nbo_32(uint32_t& n) throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 8-byte integer from the channel
  // Throws Error on failure or EOF
  uint64_t read_nbo_64() throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order 8-byte double from the socket
  // Throws Error on failure or EOF
  double read_nbo_double() throw (Error);

  //--------------------------------------------------------------------------
  // Little-endian equivalents of the above
  // Used only for external protocols specified that way
  uint16_t read_le_16() throw (Error);
  uint32_t read_le_24() throw (Error);
  uint32_t read_le_32() throw (Error);
  bool read_le_32(uint32_t& n) throw (Error);
  uint64_t read_le_64() throw (Error);
  double read_le_double() throw (Error);

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
  // Write a network byte order 8-byte double to the channel
  // Throws Error on failure or EOF
  void write_nbo_double(double f) throw (Error);

  //--------------------------------------------------------------------------
  // Little-endian (LSB first) versions of the above
  // Not recommended for new protocols - only for compatibility with existing
  // little-endian (often by default through using C structures on x86) 
  // protocols
  void write_le_16(uint16_t i) throw (Error);
  void write_le_24(uint32_t i) throw (Error);
  void write_le_32(uint32_t i) throw (Error);
  void write_le_64(uint64_t i) throw (Error);
  void write_le_double(double f) throw (Error);

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
// Data Queue Reader - see MT::DataQueue
class DataQueueReader: public Reader
{
private:
  MT::DataQueue& dq;

public:
  // Constructor
  DataQueueReader(MT::DataQueue& _dq): dq(_dq) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count) throw (Error);
};

//==========================================================================
// Data Queue Writer
class DataQueueWriter: public Writer
{
private:
  MT::DataQueue& dq;

public:
  // Constructor
  DataQueueWriter(MT::DataQueue& _dq): dq(_dq) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count) throw (Error);
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
// Bitstream reader - wraps around any Reader (bit.cc)
// Note: Bits read MSB first
class BitReader
{
private:
  Reader& reader;
  int bits_valid;
  int current_byte;

public:
  //--------------------------------------------------------------------------
  // Constructor  
  BitReader(Reader& _reader): 
    reader(_reader), bits_valid(0), current_byte(0) {}

  //--------------------------------------------------------------------------
  // Read a single bit from the channel, returning an integer
  // Throws Error on failure or EOF
  int read_bit() throw (Error);

  //--------------------------------------------------------------------------
  // Read a single bit from the channel, returning a boolean
  // Throws Error on failure or EOF
  bool read_bool() throw (Error) { return read_bit()?true:false; }

  //--------------------------------------------------------------------------
  // Read up to 32 bits from the channel
  // Returns bits in LSB of integer returned
  // Throws Error on failure or EOF
  uint32_t read_bits(int n) throw (Error);
};

//==========================================================================
// Bitstream writer - wraps around any Writer (bit.cc)
// Note: Bits written MSB first
class BitWriter
{
private:
  Writer& writer;
  int bits_valid;
  int current_byte;

public:
  //--------------------------------------------------------------------------
  // Constructor  
  BitWriter(Writer& _writer): 
    writer(_writer), bits_valid(0), current_byte(0) {}

  //--------------------------------------------------------------------------
  // Write a single bit to the channel
  // Throws Error on failure or EOF
  void write_bit(int bit) throw (Error);

  //--------------------------------------------------------------------------
  // Write a single bit to the channel as a boolean
  // Throws Error on failure or EOF
  void write_bool(bool bit) throw (Error) { write_bit(bit?1:0); }

  //--------------------------------------------------------------------------
  // Write up to 32 bits to the channel
  // Writes bits from LSB of integer given
  // Throws Error on failure or EOF
  void write_bits(int n, uint32_t bits) throw (Error);

  //--------------------------------------------------------------------------
  // Flush remaining bits (if any) as a final byte, padding with zeros
  // Throws Error on failure or EOF
  void flush() throw (Error);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CHAN_H



