//==========================================================================
// ObTools::Time: date-interval.cc
//
// Date interval functions
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include "ot-time.h"
#include "ot-text.h"
#include <stdlib.h>

namespace ObTools { namespace Time {

//--------------------------------------------------------------------------
// Get unit from a string
DateInterval::Unit DateInterval::get_unit(const string& str)
{
  if (str.empty()) return invalid;
  switch (tolower(str[0]))
  {
    case 'd': return days;
    case 'w': return weeks;
    case 'm': return months;
    case 'y': return years;
    default: return invalid;
  }
}

//--------------------------------------------------------------------------
// Construct from a string
DateInterval::DateInterval(const string& str)
{
  auto words = Text::split_words(str);
  if (words.size() == 2)
  {
    number = Text::stoi(words[0]);
    unit = get_unit(words[1]);
  }
}

//--------------------------------------------------------------------------
// Convert to a string
string DateInterval::str() const
{
  string s = Text::itos(number)+" ";

  switch (unit)
  {
    case days:   return s+((number==1)?"day":"days");
    case weeks:  return s+((number==1)?"week":"weeks");
    case months: return s+((number==1)?"month":"months");
    case years:  return s+((number==1)?"years":"years");
    default: return "INVALID";
  }
}

//--------------------------------------------------------------------------
// << operator to write DateInterval to ostream
ostream& operator<<(ostream& s, const DateInterval& di)
{
  s << di.str();
  return s;
}

}} // namespaces
