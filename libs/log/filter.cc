//==========================================================================
// ObTools::Log: filter.cc
//
// Log message filters
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Log {

//==========================================================================
// LevelFilter

void LevelFilter::log(Message& msg)
{
  if (msg.level <= level) 
    next.log(msg);
}
  
//==========================================================================
// PatternFilter

void PatternFilter::log(Message& msg)
{
  if (Text::pattern_match(pattern, msg.text))
    next.log(msg);
}
  
//==========================================================================
// Timestamp filter

void TimestampFilter::log(Message& msg)
{
  MT::Lock lock(mutex);  // localtime is dubiously thread-safe
  char stm[81];
  
  strftime(stm, 80, format.c_str(), localtime(&msg.timestamp));

  string nmsg(stm);
  nmsg += msg.text;
  msg.text = nmsg;

  next.log(msg);
}

}} // namespaces



