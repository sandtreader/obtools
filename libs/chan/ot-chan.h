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
public:
  //--------------------------------------------------------------------------
  // Basic constructor, destructor
  Reader() {}
  virtual ~Reader() {}

  //--------------------------------------------------------------------------
  // Read as much data as is available, up to 'count' bytes
  // Returns amount read
  // Throws Error on failure (not EOF)
  virtual size_t try_read(void *buf, size_t count) throw (Error) = 0;

  //--------------------------------------------------------------------------
  // Read exact amount of data from the channel into a binary buffer
  // Returns whether successful (channel not closed)
  // Throws Error on failure
  bool read(void *buf, size_t count) throw (Error);

  //--------------------------------------------------------------------------
  // Read exact amount of data from the channel into a string
  // Whether successful - all data was read before channel closed
  // Throws Error on failure
  bool read(string& s, size_t count) throw (Error);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 4-byte integer from the channel
  // Throws SocketError on failure or EOF
  uint32_t read_nbo_int() throw (Error);

  //--------------------------------------------------------------------------
  // Ditto, but allowing the possibility of failure at EOF
  // Throws Error on non-EOF failure
  bool read_nbo_int(uint32_t& n) throw (Error);
};

//==========================================================================
// Abstract Channel Writer
class Writer
{
public:
  //--------------------------------------------------------------------------
  // Basic constructor
  Writer() {}
  virtual ~Writer() {}

  //--------------------------------------------------------------------------
  // Write exact amount of data to the channel from a binary buffer
  // Throws Error on failure
  virtual void write(const void *buf, size_t count) throw (Error) = 0;

  //--------------------------------------------------------------------------
  // Write a string to the channel
  // Throws Error on failure
  void write(const string& s) throw (Error);

  //--------------------------------------------------------------------------
  // Write the given C string to the channel
  // Throws Error on failure
  void write(const char *p) throw(Error);

  //--------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 4-byte integer to the channel
  // Throws Error on failure
  void write_nbo_int(uint32_t i) throw (Error);
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
  virtual size_t try_read(void *buf, size_t count) throw (Error);
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
  virtual void write(const void *buf, size_t count) throw (Error);
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
  virtual size_t try_read(void *buf, size_t count) throw (Error);
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
  virtual void write(const void *buf, size_t count) throw (Error);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CHAN_H



