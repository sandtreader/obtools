//==========================================================================
// ObTools::Net: stream.cc
//
// iostream-like implementations for TCP sockets
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//------------------------------------------------------------------------
// TCPStreamBuf Constructor 
  TCPStreamBuf::TCPStreamBuf(TCPSocket& _s): s(_s), bufc(-1)
{
  // use unbuffered IO, since the socket buffers anyway
  setp(0,0); 
  setg(0,0,0);
}

//------------------------------------------------------------------------
// TCPStreamBuf overflow() function
int TCPStreamBuf::overflow(int c)
{
  char cc=(char)c;
  if (s.cwrite(&cc, 1) == 1)
    return 0;
  else
    return traits_type::eof();
}

//------------------------------------------------------------------------
// TCPStreamBuf underflow() function
int TCPStreamBuf::underflow()
{
  // Force a character into the 'buffer'
  if (bufc<0) uflow();
  if (bufc>=0)
    return bufc;
  else
    return traits_type::eof();
}

//------------------------------------------------------------------------
// TCPStreamBuf uflow() function
int TCPStreamBuf::uflow()
{
  char cc;
  if (s.cread(&cc, 1) == 1)
    return bufc=cc;
  else
    return traits_type::eof();
}

}} // namespaces



