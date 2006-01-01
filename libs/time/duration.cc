//==========================================================================
// ObTools::Time: duration.cc
//
// Time duration functions
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-time.h"
#include <sstream>
#include <iomanip>
#include <math.h>

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
	    int hours = (int)(t/HOUR);
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

  int hours = (int)(rt/HOUR);
  int mins  = (int)((rt-hours*HOUR)/MINUTE);
  double secs = rt-hours*HOUR-mins*MINUTE;

  // Fudge to pad floating seconds, because setw() won't do it
  char *pad = "";
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

}} // namespaces
