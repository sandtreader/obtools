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

namespace ObTools { namespace Channel {

//==========================================================================
// TCP Socket Reader

// Read implementation
size_t TCPSocketReader::try_read(void *buf, size_t count) throw (Error)
{
  try
  {
    return s.read(buf, count);
  }
  catch (Net::SocketError se)
  {
    throw Error(se.error, se.get_string());
  }
}

//==========================================================================
// TCP Socket Writer

// Write implementation
void TCPSocketWriter::write(const void *buf, size_t count) throw (Error)
{
  try
  {
    s.write(buf, count);
  }
  catch (Net::SocketError se)
  {
    throw Error(se.error, se.get_string());
  }
}

}} // namespaces



