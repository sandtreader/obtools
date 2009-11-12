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
// ostream StreamChannel

//--------------------------------------------------------------------------
// Logging function
void StreamChannel::log(Message& msg)
{
#if !defined(_SINGLE)
  MT::Lock lock(mutex);    // ostreams are NOT thread-safe
#endif

  stream << msg.text << endl;
}


}} // namespaces



