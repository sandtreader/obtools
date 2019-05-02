//==========================================================================
// ObTools::Log: channel.cc
//
// Standard channel implementations
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"
#if !defined(PLATFORM_WINDOWS)
#include <syslog.h>
#endif

namespace ObTools { namespace Log {


//==========================================================================
// StreamChannel

//--------------------------------------------------------------------------
// Logging function
void StreamChannel::log(const Message& msg)
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
void OwnedStreamChannel::log(const Message& msg)
{
  // If stream is in bad/failed state, attempt to clear
  // This can be useful if e.g. the disk was full but now has some free space
  if (!stream->good())
    stream->clear();
  *stream << msg.text << endl;
}

#if !defined(PLATFORM_WINDOWS)
//==========================================================================
// SyslogChannel

//--------------------------------------------------------------------------
// Logging function
void SyslogChannel::log(const Message& msg)
{
  int priority;
  switch (msg.level)
  {
    case Log::Level::none:
    case Log::Level::error:   priority = LOG_ERR; break;
    case Log::Level::summary: priority = LOG_NOTICE; break;
    case Log::Level::detail:  priority = LOG_INFO; break;
    case Log::Level::debug:
    case Log::Level::dump:    priority = LOG_DEBUG; break;
  }

  syslog(priority, "%s", msg.text.c_str());
}
#endif

}} // namespaces
