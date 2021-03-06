//==========================================================================
// ObTools::Chan: tcp-chan.cc
//
// TCP Socket channels (TCPSocketReader & TCPSocketWriter)
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include "ot-net.h"

#define SKIP_BUF_SIZE 4096

namespace ObTools { namespace Channel {

//==========================================================================
// TCP Socket Reader

// Read implementation
size_t TCPSocketReader::basic_read(void *buf, size_t count)
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
  catch (const Net::SocketError& se)
  {
    throw Error(se.error, se.get_string());
  }
}

//==========================================================================
// TCP Socket Writer

// Write implementation
void TCPSocketWriter::basic_write(const void *buf, size_t count)
{
  try
  {
    s.write(buf, count);
    offset += count;
  }
  catch (const Net::SocketError& se)
  {
    throw Error(se.error, se.get_string());
  }
}

}} // namespaces



