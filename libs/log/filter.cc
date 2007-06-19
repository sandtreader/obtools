//==========================================================================
// ObTools::Log: filter.cc
//
// Log message filters
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
#if !defined(_SINGLE)
  MT::Lock lock(mutex);    // localtime is dubiously thread-safe
#endif

  // Process the string for our extensions first
  // ! If there are any more of these, do it more efficiently!
  string tmp_format = Text::subst(format, "%*L", Text::itos(msg.level));
  tmp_format = Text::subst(tmp_format, "%*S", 
			   Text::ftos(msg.timestamp.seconds(),6,3,true));

  // Now do strftime on what's left
  char stm[81];
  time_t time = msg.timestamp.time();
#if defined(__WIN32__)
  // Hope that localtime is reentrant!
  strftime(stm, 80, tmp_format.c_str(), localtime(&time));
#else
  // Know that localtime_r is reentrant!
  struct tm tm;
  strftime(stm, 80, tmp_format.c_str(), localtime_r(&time, &tm));
#endif

  string nmsg(stm);
  nmsg += msg.text;
  msg.text = nmsg;

  next.log(msg);
}

}} // namespaces



