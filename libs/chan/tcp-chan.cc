//==========================================================================
// ObTools::Chan: tcp-chan.cc
//
// TCP Socket channels (TCPSocketReader & TCPSocketWriter)
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-chan.h"
#include "ot-net.h"

#define SKIP_BUF_SIZE 4096

namespace ObTools { namespace Channel {

//==========================================================================
// TCP Socket Reader

// Read implementation
size_t TCPSocketReader::basic_read(void *buf, size_t count) throw (Error)
{
  try
  {
    size_t n;
    if (buf)
    {
      n = s.read(buf, count);
    }
    else
    {
      char temp[SKIP_BUF_SIZE];  // Skip up to this many bytes
      n = s.read(temp, count);
    }

    offset += n;
    return n;
  }
  catch (Net::SocketError se)
  {
    throw Error(se.error, se.get_string());
  }
}

//==========================================================================
// TCP Socket Writer

// Write implementation
void TCPSocketWriter::basic_write(const void *buf, size_t count) throw (Error)
{
  try
  {
    s.write(buf, count);
    offset += count;
  }
  catch (Net::SocketError se)
  {
    throw Error(se.error, se.get_string());
  }
}

}} // namespaces



