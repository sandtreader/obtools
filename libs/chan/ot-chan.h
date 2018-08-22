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

// Make our lives easier without polluting anyone else
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

//--------------------------------------------------------------------------
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
  //------------------------------------------------------------------------
  // Basic constructor, destructor
  Reader(): offset(0) {}
  virtual ~Reader() {}

  //------------------------------------------------------------------------
  // Read as much data as is available, up to 'count' bytes
  // Returns amount read, also adjusts offset
  // If 'buf' is 0, just skip as much data as possible
  // Throws Error on failure (not EOF)
  virtual size_t basic_read(void *buf, size_t count) = 0;

  //------------------------------------------------------------------------
  // Try to read an exact amount of data from the channel into a binary buffer
  // Returns false if channel goes EOF before anything is read
  // Buf may be 0 to skip data
  // Throws Error on failure, or EOF after a read
  virtual bool try_read(void *buf, size_t count);

  //------------------------------------------------------------------------
  // Read exact amount of data from the channel into a binary buffer
  // Buf may be 0 to skip data
  // Throws Error on failure
  void read(void *buf, size_t count);

  //------------------------------------------------------------------------
  // Try to read an exact amount of data from the channel into a string
  // Returns false if channel goes EOF before anything is read
  // Buf may be 0 to skip data
  // Throws Error on failure, or EOF after a read
  virtual bool try_read(string& s, size_t count);

  //------------------------------------------------------------------------
  // Read exact amount of data from the channel into a string
  // Throws Error on failure
  void read(string& s, size_t count);

  //------------------------------------------------------------------------
  // Read data into buf until EOF or limit encountered
  virtual void read_to_eof(vector<unsigned char>& buf, size_t limit);

  //------------------------------------------------------------------------
  // Read data into buf until EOF or limit encountered
  virtual void read_to_eof(byte *buf, size_t limit);

  //------------------------------------------------------------------------
  // Read data into buf until EOF
  virtual void read_to_eof(vector<unsigned char>& buf);

  //------------------------------------------------------------------------
  // Read data into string until EOF or limit encountered
  virtual void read_to_eof(string& s, size_t limit);

  //------------------------------------------------------------------------
  // Read data into string until EOF
  virtual void read_to_eof(string& s);

  //------------------------------------------------------------------------
  // Try to read a single byte from the channel
  // Throws Error on failure
  virtual bool try_read_byte(unsigned char& b);

  //------------------------------------------------------------------------
  // Read a single byte from the channel
  // Throws Error on failure or EOF
  unsigned char read_byte();

  //------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 2-byte integer from the channel
  // Throws Error on failure or EOF
  uint16_t read_nbo_16();

  //------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 3-byte integer from the channel
  // Throws Error on failure or EOF
  uint32_t read_nbo_24();

  //------------------------------------------------------------------------
  // Try to read a network byte order (MSB-first) 4-byte integer
  // Throws Error on failure
  virtual bool try_read_nbo_32(uint32_t& n);

  //------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 4-byte integer from the channel
  // Throws Error on failure or EOF
  uint32_t read_nbo_32();

  //------------------------------------------------------------------------
  // Ditto, but allowing the possibility of failure at EOF
  // Throws Error on non-EOF failure
  bool read_nbo_32(uint32_t& n);

  //------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 8-byte integer from the channel
  // Throws Error on failure or EOF
  uint64_t read_nbo_64();

  //------------------------------------------------------------------------
  // Read a network byte order 8-byte double from the socket
  // Throws Error on failure or EOF
  double read_nbo_double();

  //------------------------------------------------------------------------
  // Read a fixed-point number from the channel
  double read_nbo_fixed_point(int before_bits, int after_bits);

  //------------------------------------------------------------------------
  // Little-endian equivalents of the above
  // Used only for external protocols specified that way
  // Throws Error on failure or EOF
  uint16_t read_le_16();
  uint32_t read_le_24();
  uint32_t read_le_32();
  bool read_le_32(uint32_t& n);
  uint64_t read_le_64();
  double read_le_double();

  //------------------------------------------------------------------------
  // Get current offset
  uint64_t get_offset() { return offset; }

  //------------------------------------------------------------------------
  // Skips to EOF
  // Useful for things using LimitedReader
  virtual void skip_to_eof();

  //------------------------------------------------------------------------
  // Skip N bytes
  // Throws Error on failure or EOF
  virtual void skip(size_t n);

  //------------------------------------------------------------------------
  // Skip to given alignment (bytes) from current offset
  // Throws Error on failure or EOF
  void align(size_t n);

  //------------------------------------------------------------------------
  // Whether rewindable - not by default
  virtual bool rewindable() { return false; }

  //------------------------------------------------------------------------
  // Rewind N bytes - can't do it by default
  // Throws Error on failure or EOF
  virtual void rewind(size_t) { throw Error(2, "Can't rewind"); }

  //------------------------------------------------------------------------
  // Rewind to beginning
  // Throws Error on failure or EOF
  void rewind() { rewind(offset); }
};

//==========================================================================
// Abstract Channel Writer
class Writer
{
protected:
  uint64_t offset;

public:
  //------------------------------------------------------------------------
  // Basic constructor, destructor
  Writer(): offset(0) {}
  virtual ~Writer() {}

  //------------------------------------------------------------------------
  // Write exact amount of data to the channel from a binary buffer
  // Also adjusts offset
  // Throws Error on failure
  virtual void basic_write(const void *buf, size_t count) = 0;

  //------------------------------------------------------------------------
  // Write exact amount of data to the channel from a binary buffer
  // Aliased to match Reader::read()
  void write(const void *buf, size_t count)
  { basic_write(buf, count); }

  //------------------------------------------------------------------------
  // Write a string to the channel
  // Throws Error on failure
  void write(const string& s);

  //--------------------------------------------------------------------------
  // Write a string to the channel to a fixed-length field, either limiting to
  // the given length or padding (after) with the given character
  // Throws Error on failure
  void write_fixed(const string& s, size_t length, unsigned char pad=0);

  //------------------------------------------------------------------------
  // Write the given C string to the channel
  // Throws Error on failure
  void write(const char *p);

  //------------------------------------------------------------------------
  // Write a vector of unsigned chars to the channel
  // Throws Error on failure
  void write(const vector<unsigned char>& v);

  //------------------------------------------------------------------------
  // Write a single byte to the channel
  // Throws Error on failure
  void write_byte(unsigned char b);

  //------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 2-byte integer to the channel
  // Throws Error on failure
  void write_nbo_16(uint16_t i);

  //------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 3-byte integer to the channel
  // Throws Error on failure
  void write_nbo_24(uint32_t i);

  //------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 4-byte integer to the channel
  // Throws Error on failure
  void write_nbo_32(uint32_t i);

  //------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 8-byte integer to the channel
  // Throws Error on failure
  void write_nbo_64(uint64_t i);

  //------------------------------------------------------------------------
  // Write a network byte order 8-byte double to the channel
  // Throws Error on failure or EOF
  void write_nbo_double(double f);

  //------------------------------------------------------------------------
  // Write a network byte order fixed-point double
  // Throws Error on failure or EOF
  void write_nbo_fixed_point(double f, int before_bits, int after_bits);

  //------------------------------------------------------------------------
  // Little-endian (LSB first) versions of the above
  // Not recommended for new protocols - only for compatibility with existing
  // little-endian (often by default through using C structures on x86)
  // protocols
  // Throws Error on failure or EOF
  void write_le_16(uint16_t i);
  void write_le_24(uint32_t i);
  void write_le_32(uint32_t i);
  void write_le_64(uint64_t i);
  void write_le_double(double f);

  //------------------------------------------------------------------------
  // Get current offset
  uint64_t get_offset() { return offset; }

  //------------------------------------------------------------------------
  // Skip N bytes, writing zero
  // Throws Error on failure or EOF
  virtual void skip(size_t n);

  //------------------------------------------------------------------------
  // Pad to given alignment (bytes) from given current offset
  // Throws Error on failure or EOF
  void align(size_t n);

  //------------------------------------------------------------------------
  // Whether rewindable - not by default
  virtual bool rewindable() { return false; }

  //------------------------------------------------------------------------
  // Rewind N bytes - can't do it by default
  // Throws Error on failure or EOF
  virtual void rewind(size_t) { throw Error(2, "Can't rewind"); }

  //------------------------------------------------------------------------
  // Rewind to beginning
  // Throws Error on failure or EOF
  void rewind() { rewind(offset); }
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
  virtual size_t basic_read(void *buf, size_t count);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
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
  virtual void basic_write(const void *buf, size_t count);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
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
  virtual size_t basic_read(void *buf, size_t count);
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
  virtual void basic_write(const void *buf, size_t count);
};

//==========================================================================
// Raw file descriptor reader
class FDReader: public Reader
{
private:
  int fd;

public:
  // Constructor
  FDReader(int _fd): fd(_fd) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count);
};

//==========================================================================
// Raw file description writer
class FDWriter: public Writer
{
private:
  int fd;

public:
  // Constructor
  FDWriter(int _fd): fd(_fd) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count);
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
  BlockReader(const vector<unsigned char>& v):
    data(&v[0]), length(v.size()) {}

  // Read implementations
  virtual size_t basic_read(void *buf, size_t count);
  virtual void skip(size_t n);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
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
  BlockWriter(vector<unsigned char>& v):
    data(&v[0]), length(v.size()) {}

  // Write implementation
  virtual void basic_write(const void *buf, size_t count);
  virtual void skip(size_t n);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
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
  virtual size_t basic_read(void *buf, size_t count);
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
  virtual void basic_write(const void *buf, size_t count);
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
  virtual size_t basic_read(void *buf, size_t count);
  virtual void skip(size_t n);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
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
  virtual void basic_write(const void *buf, size_t count);
  virtual void skip(size_t n);
  virtual bool rewindable() { return true; }
  virtual void rewind(size_t n);
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
  //------------------------------------------------------------------------
  // Constructor
  BitReader(Reader& _reader):
    reader(_reader), bits_valid(0), current_byte(0) {}

  //------------------------------------------------------------------------
  // Read a single bit from the channel, returning an integer
  // Throws Error on failure or EOF
  int read_bit();

  //------------------------------------------------------------------------
  // Read a single bit from the channel, returning a boolean
  // Throws Error on failure or EOF
  bool read_bool() { return read_bit()?true:false; }

  //------------------------------------------------------------------------
  // Read up to 32 bits from the channel
  // Returns bits in LSB of integer returned
  // Throws Error on failure or EOF
  uint32_t read_bits(int n);
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
  //------------------------------------------------------------------------
  // Constructor
  BitWriter(Writer& _writer):
    writer(_writer), bits_valid(0), current_byte(0) {}

  //------------------------------------------------------------------------
  // Write a single bit to the channel
  // Throws Error on failure or EOF
  void write_bit(int bit);

  //------------------------------------------------------------------------
  // Write a single bit to the channel as a boolean
  // Throws Error on failure or EOF
  void write_bool(bool bit) { write_bit(bit?1:0); }

  //------------------------------------------------------------------------
  // Write up to 32 bits to the channel
  // Writes bits from LSB of integer given
  // Throws Error on failure or EOF
  void write_bits(int n, uint32_t bits);

  //------------------------------------------------------------------------
  // Flush remaining bits (if any) as a final byte, padding with zeros
  // Throws Error on failure or EOF
  void flush();
};

//==========================================================================
// Bitstream reader with Exp-Golomb support
// Note: Bits read MSB first
class BitEGReader: public BitReader
{
public:
  //------------------------------------------------------------------------
  // Constructor
  BitEGReader(Reader& _reader):
    BitReader(_reader) {}

  //------------------------------------------------------------------------
  // Read an Exp-Golomb coded value from the channel
  // Throws Error on failure or EOF
  uint32_t read_exp_golomb();
};

//==========================================================================
// Limited Reader
// Container for an abstract Reader that limits the amount of data that may
// be read from it
class LimitedReader: public Reader
{
private:
  Reader& reader;
  size_t left;

public:
  //------------------------------------------------------------------------
  // Constructor
  LimitedReader(Reader& _reader, size_t limit):
    reader(_reader), left(limit)
  {}

  //------------------------------------------------------------------------
  // Read as much data as is available, up to 'count' bytes
  // Returns amount read, also adjusts offset
  // If 'buf' is 0, just skip as much data as possible
  // Throws Error on failure (not EOF)
  size_t basic_read(void *buf, size_t count)
  {
    if (count > left)
      count = left;
    size_t r = reader.basic_read(buf, count);
    left -= r;
    offset += r;
    return r;
  }

  //------------------------------------------------------------------------
  // Try to read an exact amount of data from the channel into a binary buffer
  // Returns false if channel goes EOF before anything is read
  // Buf may be 0 to skip data
  // Throws Error on failure, or EOF after a read
  bool try_read(void *buf, size_t count)
  {
    if (!left)
      return false;
    return Reader::try_read(buf, count);
  }

  //------------------------------------------------------------------------
  // Try to read an exact amount of data from the channel into a string
  // Returns false if channel goes EOF before anything is read
  // Buf may be 0 to skip data
  // Throws Error on failure, or EOF after a read
  bool try_read(string& s, size_t count)
  {
    if (!left)
      return false;
    return Reader::try_read(s, count);
  }

  //------------------------------------------------------------------------
  // Read data into buf until EOF or limit encountered
  void read_to_eof(vector<unsigned char>& buf, size_t limit)
  {
    Reader::read_to_eof(buf, min(left, limit));
  }

  //------------------------------------------------------------------------
  // Read data into buf until EOF
  void read_to_eof(vector<unsigned char>& buf)
  {
    Reader::read_to_eof(buf, left);
  }

  //------------------------------------------------------------------------
  // Read data into string until EOF or limit encountered
  void read_to_eof(string& s, size_t limit)
  {
    Reader::read_to_eof(s, min(left, limit));
  }

  //------------------------------------------------------------------------
  // Read data into string until EOF
  void read_to_eof(string& s)
  {
    Reader::read_to_eof(s, left);
  }

  //------------------------------------------------------------------------
  // Try to read a network byte order (MSB-first) 4-byte integer
  // Throws Error on failure
  bool try_read_nbo_32(uint32_t& n)
  {
    if (!left)
      return false;
    return Reader::try_read_nbo_32(n);
  }

  //------------------------------------------------------------------------
  // Try to read a single byte from the channel
  // Throws Error on failure
  bool try_read_byte(unsigned char& b)
  {
    if (!left)
      return false;
    return Reader::try_read_byte(b);
  }

  //------------------------------------------------------------------------
  // Skips to EOF
  void skip_to_eof()
  {
    if (left)
      reader.skip(left);
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CHAN_H
