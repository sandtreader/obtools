//==========================================================================
// ObTools::Log: logstream.cc
//
// Log ostream handler
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//------------------------------------------------------------------------
// LogStreamBuf Constructor 
LogStreamBuf::LogStreamBuf(Channel& _channel, Level _level): 
  channel(_channel), level(_level), closed(false)
{
  // use unbuffered IO, since we're going to buffer it anyway
  setp(0,0); 
  setg(0,0,0);
}

//------------------------------------------------------------------------
// LogStreamBuf Destructor - force close
LogStreamBuf::~LogStreamBuf() 
{ 
  close(); 
}

//------------------------------------------------------------------------
// LogStreamBuf close() function
// Does all the real work here!
void LogStreamBuf::close()
{
  // Avoid doing this more than once
  if (closed) return;
  closed = true;

  Message msg(level, buffer);
  channel.log(msg);
}

//------------------------------------------------------------------------
// LogStreamBuf overflow() function
// Handles characters one at a time, since streambuf is unbuffered
int LogStreamBuf::overflow(int c)
{
  if (c==EOF)
    close();
  else
    buffer += (char)c;

  return 0;
}

}} // namespaces



