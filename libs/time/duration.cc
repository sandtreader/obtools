//==========================================================================
// ObTools::Time: duration.cc
//
// Time duration functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-time.h"
#include "ot-text.h"
#include <sstream>
#include <iomanip>
#include <math.h>
#include <time.h>

namespace ObTools { namespace Time {

namespace {
// Unit dictionary - just one static instance of this
struct UnitDictionary
{
  map<string, int> multiples =
  {
    {"s",         1},
    {"sec",       1},
    {"secs",      1},
    {"second",    1},
    {"seconds",   1},

    {"m",         MINUTE},
    {"min",       MINUTE},
    {"mins",      MINUTE},
    {"minute",    MINUTE},
    {"minutes",   MINUTE},

    {"h",         HOUR},
    {"hr",        HOUR},
    {"hrs",       HOUR},
    {"hour",      HOUR},
    {"hours",     HOUR},

    {"d",         DAY},
    {"day",       DAY},
    {"days",      DAY},
    {"dt",        DAY}, // For ISO P1DT12H form

    {"w",         WEEK},
    {"wk",        WEEK},
    {"wks",       WEEK},
    {"week",      WEEK},
    {"weeks",     WEEK},
  };
  map<string, int> fractions =
  {
    {"ns",        NANO},
    {"us",        MICRO},
    {"ms",        MILLI},
  };
};
} // anonymous namespace

//--------------------------------------------------------------------------
// Constructor from string
// See ot-time.h for details
Duration::Duration(const string& text)
{
  static const auto units = UnitDictionary{};
  istringstream iss(text);
  int shift=0;
  char c=0;
  bool iso = false;
  t = 0;

  // Check for ISO format with P, and ignore it (note u/c only)
  iss >> c;
  if (c=='P')
    iso = true;
  else
    iss.putback(c);

  // Loop over multiple units
  while (!iss.fail() && !iss.eof())
  {
    // In ISO we might see a 'T' separator (which is redundant)
    if (iso)
    {
      iss >> c;
      if (c!='T') iss.putback(c);
    }

    // One way or another, we need a number, which might be a float
    double f=0;
    iss >> f;

    if (!iss.eof())
    {
      // Check next character
      iss >> c;  // NB - WS stripped

      if (c==':')
      {
        // Add as seconds (for now)
        t+=f;

        // Check shift number
        switch (++shift)
        {
          case 1:
          case 2:  // First two are easy:
            t*=60;
            break;

          case 3:  // Third is harder - hours moving to days
          {
            int hours = static_cast<int>(t/HOUR);
            double minsecs = t-hours*HOUR;
            t=DAY*hours + MINUTE*minsecs;
          }
          break;

          default: return; // Too many - stop
        }
      }
      else
      {
        // Time unit - read a word
        iss.putback(c);

        // Read an alpha word
        string word;
        while (isalpha(c = iss.get()))
          word += c;
        iss.putback(c);

        // We accept upper but test lower case
        word = Text::tolower(word);

        // Look up word in units dictionary
        map<string, int>::const_iterator p;
        p = units.multiples.find(word);
        if (p!=units.multiples.end())
        {
          // Multiply by value
          t+=f*p->second;
        }
        else
        {
          p = units.fractions.find(word);
          if (p!=units.fractions.end())
          {
            // Divide by value
            t+=f/p->second;
          }
          else
          {
            // Unrecognised word - use as seconds and bomb out
            t+=f;
            return;
          }
        }
      }
    }
    else
    {
      // Finished
      t+=f;
    }
  }
}

//--------------------------------------------------------------------------
// Convert to HH:MM:SS string - never goes into days or higher
// Can output fractional seconds - rounds to nearest millisecond
string Duration::hms() const
{
  ostringstream oss;
  // Round seconds to milliseconds, to avoid unfortunate combinations
  // (e.g.) 59.999999 -> 00:00:60
  double rt = floor(t*MILLI+0.5)/MILLI;

  int hours = static_cast<int>(rt/HOUR);
  int mins  = static_cast<int>((rt-hours*HOUR)/MINUTE);
  double secs = rt-hours*HOUR-mins*MINUTE;

  // Fudge to pad floating seconds, because setw() won't do it
  const char *pad = "";
  if (secs < 10) pad = "0";

  // Fix precision for small values
  int prec = 11;
  if (secs < 10) prec--;
  if (secs < 1) prec--;

  oss << setw(2) << setfill('0') << hours << ':'
      << setw(2) << setfill('0') << mins << ':'
      << pad << setprecision(prec) << secs;

  return oss.str();
}

//--------------------------------------------------------------------------
// Convert to ISO duration string
// Generates P[n]DT[n]H[n]M[n]S form or empty if invalid
// This format is also compatible with XML
// Note: Never generates months or years because they are variable
string Duration::iso() const
{
  // Round seconds to milliseconds, to avoid unfortunate combinations
  // (e.g.) 59.999999 -> 00:00:60
  double rt = floor(t*MILLI+0.5)/MILLI;

  if (rt == 0.0)
    return "P0D";

  int days = static_cast<int>(rt / (HOUR * 24));
  rt -= days * HOUR * 24;
  int hours = static_cast<int>(rt / HOUR);
  rt -= hours * HOUR;
  int minutes = static_cast<int>(rt / MINUTE);
  rt -= minutes * MINUTE;
  double secs = rt;

  ostringstream oss;
  oss << 'P';
  if (days)
    oss << days << 'D';
  if (hours || minutes || secs)
  {
    oss << 'T';
    if (hours)
      oss << hours << 'H';
    if (minutes)
      oss << minutes << 'M';
    if (secs)
    {
      int precision = 2;
      if (secs > 1)
        ++precision;
      if (secs > 10)
        ++precision;
      oss << setprecision(precision) << secs << 'S';
    }
  }

  return oss.str();
}

//------------------------------------------------------------------------
// Convert to unit-based string - e.g. "1 hour", "3 min 4 sec"
// Note, doesn't handle fractional seconds, only goes up to days.
string Duration::unit() const
{
  ostringstream oss;
  // Round seconds to milliseconds, to avoid unfortunate combinations
  // (e.g.) 59.999999 -> 00:00:60
  double rt = floor(t*MILLI+0.5)/MILLI;

  int days = static_cast<int>(rt/DAY);
  rt -= days*DAY;
  int hours = static_cast<int>(rt/HOUR);
  rt -= hours*HOUR;
  int mins  = static_cast<int>(rt/MINUTE);
  rt -= mins*MINUTE;
  int secs = static_cast<int>(rt);

  if (days) oss << days << " day" << (days>1?"s":"") << " ";
  if (hours) oss << hours << " hour" << (hours>1?"s":"") << " ";
  if (mins) oss << mins << " min ";
  if (secs) oss << secs << " sec";
  return Text::canonicalise_space(oss.str());
}

//--------------------------------------------------------------------------
// Constructor-like static function to return monotonic clock - baseline
// unknown, but guaranteed never to get mangled by ntpd, DST et al.
// Returns Duration(0.0) if clock not available
Duration Duration::clock()
{
  // Don't use monotonic clock in single threaded mode, to avoid pulling
  // in librt
#if defined(CLOCK_MONOTONIC) && !defined(_SINGLE)
  struct timespec ts;
  if (!clock_gettime(CLOCK_MONOTONIC, &ts))
    return Duration(static_cast<double>(ts.tv_sec) +
                    static_cast<double>(ts.tv_nsec)/1.0e9);
#endif

  return Duration();
}

//--------------------------------------------------------------------------
// Arithmetic operators the other way
Duration operator*(double n, const Duration& d)
{
  return d * n;
}

}} // namespaces
