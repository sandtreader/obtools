//==========================================================================
// ObTools::Time: split.cc
//
// Split time functions
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include "ot-time.h"
#include "ot-text.h"
#include <stdlib.h>

namespace ObTools { namespace Time {

//------------------------------------------------------------------------
// Normalise negative or out-of-range numbers
void Split::normalise()
{
  while (sec < 0)   { min--;  sec+=60; }
  while (sec > 59)  { min++;  sec-=60; }
  while (min < 0)   { hour--; min+=60; }
  while (min > 59)  { hour++; min-=60; }
  while (hour < 0)  { day--;  hour+=24; }
  while (hour > 23) { day++;  hour-=24; }

  // We do months *first* so we have a valid month to shift days with
  // - the day fix will handle any month over/underflow anyway
  while (month < 1)  { year--; month+=12; }
  while (month > 12) { year++; month-=12; }

  // Days we have to do the full works, because months are variable length
  if (day < 1)
  {
    Time::Duration diff((1-day) * DAY);
    day = 1;
    (Time::Stamp(*this) - diff).split(*this);
  }
  else if (day > 28)  // May trigger unnecessarily, but we have no way to know
  {
    Time::Duration diff((day-1) * DAY);
    day = 1;
    (Time::Stamp(*this) + diff).split(*this);
  }
}

//--------------------------------------------------------------------------
// << operator to write Split to ostream
ostream& operator<<(ostream& s, const Split& sp)
{
  s << sp.year << "-" << sp.month << "-" << sp.day
    << " " << sp.hour << ":" << sp.min << ":" << sp.sec;
  return s;
}

}} // namespaces
