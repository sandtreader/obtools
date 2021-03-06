//==========================================================================
// ObTools::Net: stream.cc
//
// iostream-like implementations for TCP sockets
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// TCPStreamBuf Constructor
TCPStreamBuf::TCPStreamBuf(TCPSocket& _s, int _in_buf_size, int _out_buf_size):
  s(_s),
  in_buf_size(_in_buf_size), in_buf(0),
  out_buf_size(_out_buf_size), out_buf(0)
{
  // Must always have a one-character input buffer
  if (!in_buf_size) in_buf_size = 1;
  in_buf = new char[in_buf_size];
  setg(in_buf, in_buf+in_buf_size, in_buf+in_buf_size);

  // And a one-character output buffer, although we tell the streambuf
  // there is one less than there really is, so there's always room for
  // the extra character in overflow()
  if (!out_buf_size) out_buf_size = 1;
  out_buf = new char[out_buf_size];
  setp(out_buf, out_buf+out_buf_size-1);
}

//--------------------------------------------------------------------------
// TCPStreamBuf overflow() function
// Sends out and clears buffer, plus character given
int TCPStreamBuf::overflow(int c)
{
  char *p = pptr();

  // We know we can always add an extra character, because we advertise
  // one less than we have
  if (c != traits_type::eof()) *p++ = static_cast<char>(c);

  // Now send out what we have
  int n = s.cwrite(out_buf, p-out_buf);

  // Return EOF if it failed
  if (n != p-out_buf) return traits_type::eof();

  // Reset buffer
  setp(out_buf, out_buf+out_buf_size-1);
  return 0;
}

//--------------------------------------------------------------------------
// TCPStreamBuf sync() function
// Synchronises output
int TCPStreamBuf::sync()
{
  // Send out if we need to
  if (pptr() > pbase()) overflow(traits_type::eof());
  return 0;
}

//--------------------------------------------------------------------------
// TCPStreamBuf underflow() function
// Refill buffers and return next character (but do not remove it)
int TCPStreamBuf::underflow()
{
  // Read a buffer full
  int n = s.cread(in_buf, in_buf_size);
  if (n>0)
  {
    setg(in_buf, in_buf, in_buf+n);
    return *reinterpret_cast<unsigned char *>(in_buf); // Avoid sign extension
                                                       // generating EOF!
  }
  else return traits_type::eof();
}

//--------------------------------------------------------------------------
// TCPStreamBuf uflow() function
int TCPStreamBuf::uflow()
{
  int c = underflow();
  if (c != traits_type::eof()) gbump(1);  // Move over it
  return c;
}

//--------------------------------------------------------------------------
// TCPStreamBuf showmanyc() function
streamsize TCPStreamBuf::showmanyc()
{
  return egptr()-gptr();
}

//--------------------------------------------------------------------------
// TCPStreamBuf Destructor
TCPStreamBuf::~TCPStreamBuf()
{
  if (in_buf) delete[] in_buf;
  if (out_buf) delete[] out_buf;
}


}} // namespaces



