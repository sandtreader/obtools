//==========================================================================
// ObTools::Log: distributor.cc
//
// Log distributor
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//--------------------------------------------------------------------------
// Connect a channel (takes ownership)
void Distributor::connect(Channel *channel)
{
  MT::Lock lock(mutex);
  channels.emplace_back(channel);
}

//--------------------------------------------------------------------------
// Connect channel with timestamp and level logging (takes ownership)
void Distributor::connect_full(Channel *channel, Level level,
                               const string& time_format)
{
  auto fc = new FilteredChannel{channel};
  fc->append_filter(new LevelFilter{level});
  fc->append_filter(new TimestampFilter{time_format});
  connect(fc);
}

//--------------------------------------------------------------------------
// Log a message
void Distributor::log(Message& msg)
{
  MT::Lock lock(mutex);
  // Send to all channels
  for (auto& channel: channels)
    channel->log(msg);
}

}} // namespaces
