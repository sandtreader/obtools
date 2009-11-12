//==========================================================================
// ObTools::Log: logstream.cc
//
// Log ostream handler
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//------------------------------------------------------------------------
// StreamBuf Constructor 
StreamBuf::StreamBuf(Channel& _channel, Level _level): 
  closed(false), channel(_channel), level(_level)
{
  // use unbuffered IO, since we're going to buffer it anyway
  setp(0,0); 
  setg(0,0,0);
}

//------------------------------------------------------------------------
// StreamBuf close() function
// Does all the real work here!
void StreamBuf::close()
{
  // Avoid doing this more than once
  if (closed) return;
  closed = true;

  Message msg(level, buffer);
  channel.log(msg);
}

//------------------------------------------------------------------------
// StreamBuf overflow() functitcon
// Handles characters one at a time, since streambuf is unbuffered
int StreamBuf::overflow(int c)
{
  if (c==EOF)
    close();
  else if (c=='\n')
  {
    // Pass buffer on at EOL, without EOL in it
    Message msg(level, buffer);
    channel.log(msg);
    buffer.erase();
  }
  else buffer += (char)c;

  return 0;
}

}} // namespaces



