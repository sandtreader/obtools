//==========================================================================
// ObTools::Time: duration.cc
//
// Time duration functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-time.h"
#include <sstream>
#include <iomanip>
#include <math.h>
#include <time.h>

namespace ObTools { namespace Time {

//------------------------------------------------------------------------
// Constructor (static initialiser) of unit dictionary
Duration::UnitDictionary::UnitDictionary()
{
  multiples["s"]       = 1;
  multiples["sec"]     = 1;
  multiples["secs"]    = 1;
  multiples["second"]  = 1;
  multiples["seconds"] = 1;
 
  multiples["m"]       = MINUTE;
  multiples["min"]     = MINUTE;
  multiples["mins"]    = MINUTE;
  multiples["minute"]  = MINUTE;
  multiples["minutes"] = MINUTE;

  multiples["h"]       = HOUR;
  multiples["hour"]    = HOUR;
  multiples["hours"]   = HOUR;

  multiples["d"]       = DAY;
  multiples["day"]     = DAY;
  multiples["days"]    = DAY;

  multiples["w"]       = WEEK;
  multiples["week"]    = WEEK;
  multiples["weeks"]   = WEEK;

  fractions["ns"]      = NANO;
  fractions["us"]      = MICRO;
  fractions["ms"]      = MILLI;
}

// Actual static variable
Duration::UnitDictionary Duration::units;

//------------------------------------------------------------------------
// Constructor from string
// See ot-time.h for details
Duration::Duration(const string& text)
{
  istringstream iss(text);
  int shift=0;

  t = 0;
  while (!iss.fail() && !iss.eof())
  {
    // One way or another, we need a number, which might be a float
    double f=0;
    iss >> f;

    if (!iss.eof())
    {
      // Check next character
      char c=0;
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

	string word;
	iss >> word;

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

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
// Convert to ISO duration string
// Generates P[n]Y[n]M[n]DT[n]H[n]M[n]S form or empty if invalid
// This format is also compatible with XML
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

//------------------------------------------------------------------------
// << operator to write Duration to ostream
ostream& operator<<(ostream& s, const Duration& d)
{
  s << d.hms();
  return s;
}

}} // namespaces
