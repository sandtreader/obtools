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
// Pattern filter
void PatternFilter::log(const Message& msg)
{
  if (Text::pattern_match(pattern, msg.text))
    next->log(msg);
}

//==========================================================================
// Timestamp filter
void TimestampFilter::log(const Message& msg)
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

  Message nmsg(msg.level, msg.timestamp, stm+msg.text);
  next->log(nmsg);
}

//==========================================================================
// RepeatedMessage filter
void RepeatedMessageFilter::log(const Message& msg)
{
  bool same = msg.text == last_msg.text;
  bool within_hold_time = msg.timestamp-last_msg.timestamp < hold_time;
  if (same)
  {
    repeats++;
    last_repeat_timestamp = msg.timestamp;
  }

  if (!same || !within_hold_time)
  {
    if (repeats > 1)
    {
      Message report(msg.level, last_repeat_timestamp,
                     "("+Text::itos(repeats)+" identical messages suppressed)");
      next->log(report);
      repeats = 0;
    }
    else if (repeats == 1)
    {
      // Just re-output the last message, with timestamp fixed
      Message omsg(last_msg.level, last_repeat_timestamp, last_msg.text);
      next->log(omsg);
      repeats = 0;
    }

    if (!same) next->log(msg);
    last_msg = msg;
  }
}

}} // namespaces
