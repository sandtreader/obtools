//==========================================================================
// ObTools::Log: channel.cc
//
// Standard channel implementations
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {


//==========================================================================
// StreamChannel

//--------------------------------------------------------------------------
// Logging function
void StreamChannel::log(Message& msg)
{
  // If stream is in bad/failed state, attempt to clear
  // This can be useful if e.g. the disk was full but now has some free space
  if (!stream->good())
    stream->clear();
  *stream << msg.text << endl;
}

//==========================================================================
// OwnedStreamChannel

//--------------------------------------------------------------------------
// Logging function
void OwnedStreamChannel::log(Message& msg)
{
  // If stream is in bad/failed state, attempt to clear
  // This can be useful if e.g. the disk was full but now has some free space
  if (!stream->good())
    stream->clear();
  *stream << msg.text << endl;
}


}} // namespaces
