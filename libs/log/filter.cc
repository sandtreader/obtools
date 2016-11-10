//==========================================================================
// ObTools::Log: filter.cc
//
// Log message filters
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"
#include "ot-text.h"
#include <math.h>

namespace ObTools { namespace Log {

//==========================================================================
// Filtered Channel
void FilteredChannel::log(Message& msg)
{
  for (auto& filter: filters)
  {
    if (!filter->pass(msg))
      return;
  }
  output->log(msg);
}

//==========================================================================
// Pattern filter
bool PatternFilter::pass(Message& msg)
{
  return Text::pattern_match(pattern, msg.text);
}

//==========================================================================
// Timestamp filter

bool TimestampFilter::pass(Message& msg)
{
  // Process the string for our extensions first
  // ! If there are any more of these, do it more efficiently!
  string tmp_format = Text::subst(format, "%*L",
                                  Text::itos(static_cast<int>(msg.level)));
  double seconds = msg.timestamp.seconds();

  // Floor to nearest millisecond to prevent 60th second ever happening
  seconds = floor(seconds*1000.0)/1000.0;
  tmp_format = Text::subst(tmp_format, "%*S", Text::ftos(seconds,6,3,true));

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

  return true;
}

}} // namespaces
