//==========================================================================
// ObTools::Log: channel.cc
//
// Standard channel implementations
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {


//==========================================================================
// ostream StreamChannel

//--------------------------------------------------------------------------
// Logging function
void StreamChannel::log(Message& msg)
{
  stream << msg.text << endl;
}


}} // namespaces



