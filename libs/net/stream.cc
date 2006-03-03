//==========================================================================
// ObTools::Net: stream.cc
//
// iostream-like implementations for TCP sockets
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================


// !!! This is very ugly, inefficient, and needs implementing with proper 
// buffers!

#include "ot-net.h"

namespace ObTools { namespace Net {

//------------------------------------------------------------------------
// TCPStreamBuf Constructor 
TCPStreamBuf::TCPStreamBuf(TCPSocket& _s): s(_s), bufc(-1), held(false)
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
  if (held)
  {
    held = false;
    return bufc;
  }

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
  if (held)
  {
    held = false;
    return bufc;
  }
  else if (s.cread(&cc, 1) == 1)
    return bufc=cc;
  else
  {
    bufc = -1;
    return traits_type::eof();
  }
}

//------------------------------------------------------------------------
// TCPStreamBuf pbackfail() function
// We need this otherwise putback doesn't work with no buffering
int TCPStreamBuf::pbackfail(int c)
{
  if (held)
    return traits_type::eof();
  else
  {
    held = true;
    return bufc=c;
  }
}


}} // namespaces



