//==========================================================================
// ObTools::Time: stamp.cc
//
// Time stamp functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-time.h"
#include "stdlib.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#if defined(__WIN32__)
#include <windows.h>
// Widely-quoted difference between Windows filetime and Unix time_t epochs
#define FILETIME_UNIX_EPOCH_DIFF 11644473600ULL
#include <time.h>
#else
#include <sys/time.h>
#endif

namespace ObTools { namespace Time {

// Look up table of cumulative days at start of each month (non leap-years)
static const int monthdays[] = {   0,  31,  59,  90, 120, 151, 
                                 181, 212, 243, 273, 304, 334 };

//------------------------------------------------------------------------
// Helper function to convert a split time into an internal timestamp
ntp_stamp_t Stamp::combine(const Split& split)
{
  // One would have thought you could use the standard C library time
  // functions for this, but there is no portable way I know of for
  // doing a mktime to get a UTC output (in other words, as localtime
  // is to mktime, gmtime is to ...).  Since we're totally
  // uninterested in what this machine's TZ is, it seems safer to do
  // it ourselves...

  // Accumulate seconds initially, then worry about shifting up to NTP
  // at the end - time_t should be safe for this
  time_t seconds;
 
  // First work out leapdays since 1900 - one every 4, lose one every 100, 
  // gain one every 400 from 2000.  Note - we're using year from zero here!
  // Also, remember this includes the current year as well.  For this purpose, 
  // if the month is Jan or Feb, put us back a year so we don't include the
  // leap day (if there is one)
  int ldyear = split.year;
  if (split.month < 3) ldyear--;

  int leapdays = ldyear/4;
  leapdays -= ldyear/100;
  leapdays += ldyear/400;

  // So, that's the number of leaps there have been since 0AD, assuming
  // they used the Gregorian calendar all that time - but how many since
  // 1900, then?  We need to know how many happened (would have happened)
  // up to 1900... It's magic number, left as an exercise for the reader...
  leapdays -= 460;
  seconds = ((split.year-1900) * 365 + leapdays) * DAY;

  // Check month and convert
  if (split.month<1 || split.month>12) return 0;
  seconds += monthdays[split.month-1]*DAY;

  // All the rest are simple, thankfully!
  seconds += (split.day-1)*DAY;  // Note, day 1 is zero offset
  seconds += split.hour*HOUR;
  seconds += split.min*MINUTE;

  // Now upshift this to NTP form and add float seconds (do it in this
  // order to preserve precision)
  ntp_stamp_t ts = (ntp_stamp_t)seconds << NTP_SHIFT;
  ts += (ntp_stamp_t)(split.sec * (1ULL << NTP_SHIFT));

  return ts;
}

//------------------------------------------------------------------------
// Helper function to split an internal timestamp into a split time
Split Stamp::split(ntp_stamp_t ts)
{
  // For this we could use gmtime, in theory, but it's not threadsafe!
  // So, we're back to doing it ourselves...
  Split sp;

  // First we downgrade to integer to make life easier - we'll add 
  // back the fractional part later.  
  unsigned long seconds = (unsigned long)(ts>>NTP_SHIFT); 

  // Get estimate of years - near enough for NTP validity timeframe
  int years = seconds/(365.24*DAY); 
  int leapdays = years/4;
  leapdays-=years/100;             // Chop off centuries
  leapdays+=(years+300)/400;       // Add back 400's, allowing for 1900 start

  // But beware!  If this is at the beginning of a leapyear, we will have
  // wrongly included the leapday for this year, so we need to discount it
  bool early = false;
  if (!(years%4) && ((years%100) || !((years+300)%400))  // We're a leapyear
      && seconds-(years*365+leapdays-1)*DAY < DAY*59)    // Before Feb 29
  {
    early = true;                                        // See below
    leapdays--;                                          // Ignore this one
  }

  // Now delete leapdays
  seconds -= leapdays*DAY;

  // Now we can do the year calculation again with corrected time
  sp.year = seconds/(365*DAY);
  seconds -= sp.year*(365*DAY);
  sp.year += 1900;

  // Get year day (0-365), and find month it's in - but think about whether
  // it's a leap year and fix up dates around the 28th.
  int yday = seconds/DAY;
  bool isleapyear = !(sp.year%4) && ((sp.year%100) || !(sp.year%400));

  int i;
  for(i=1; i<12; i++)
    if (yday < monthdays[i]) break;
  sp.month = i;

  // Remove months up to here
  seconds -= monthdays[i-1]*DAY;

  // Now it's easy(-ish)
  sp.day = seconds/DAY;
  seconds -= sp.day*DAY;
  sp.day++;  // From 1

  // Fix up leapday itself
  // Now things are a bit tricky - there could be two day 58's in a leap year
  //   1) The original 28th Feb
  //   2) The original 29th Feb, now pulled back because of the leapday
  // Happily, only (1) has 'early' set here - (2) is the real leapday
  if (isleapyear && !early && yday == 58)
  {
    sp.month = 2;
    sp.day = 29;
  }

  // Hours & minutes
  sp.hour = seconds/HOUR;
  seconds -= sp.hour*HOUR;

  sp.min = seconds/MINUTE;
  seconds -= sp.min*MINUTE;

  // Get integer part of seconds, then add back float part by masking off
  // lower 32 bits and dividing down
  sp.sec = seconds;
  sp.sec += (double)(ts & ((1ULL<<NTP_SHIFT)-1))/(1ULL<<NTP_SHIFT);

  return sp;
}

//------------------------------------------------------------------------
// Constructor from string
// See ot-time.h for details
namespace {
  const int OT_NO_DATA = -1;  // Avoid Windows NO_DATA macro! :-(
  const int OT_BAD_DATA = -2;

  int read_part(const string& text, string::size_type& pos, int length)
  {
    if (text.size() == pos)
      return OT_NO_DATA;

    if (text.size() < pos + length)
      return OT_BAD_DATA;

    string s(text, pos, length);
    for (string::const_iterator it = s.begin(); it != s.end(); ++it)
      if (*it < '0' || *it > '9')
        return OT_BAD_DATA;

    pos += length;
    return atoi(s.c_str());
  }

  // not general - specific for float seconds with optional Z at end
  double read_part_f(const string& text, string::size_type& pos, int length)
  {
    if (text.size() == pos)
      return OT_NO_DATA;

    if (length < 2)
      return OT_BAD_DATA;

    if (text.size() < pos + length)
      return OT_BAD_DATA;

    string s(text, pos, length);

    if (s.size() > 2 && *s.rbegin() == 'Z')
      s.erase(s.size() - 1, 1);

    int dp_count = 0;
    for (string::const_iterator it = s.begin(); it != s.end(); ++it)
    {
      if (*it == '.')
        dp_count++;
      else if (*it < '0' || *it > '9')
        return OT_BAD_DATA;
    }

    if (dp_count > 1)
      return OT_BAD_DATA;

    pos += length;
    return atof(s.c_str());
  }

  bool read_filler(const string& text, string::size_type& pos, char c)
  {
    if (text.size() > pos)
    {
      if (text[pos] == c)
      {
        pos++;
        return true;
      }
    }
    return false;
  }
}
Stamp::Stamp(const string& text, bool lenient)
{
  string::size_type pos=0;

  // Clear stamp incase we fail
  t=0;

  // Accumulate a Split structure to convert later
  Split split;

  // Read year
  int y = read_part(text, pos, 4);
  if (y < 0) return;
  split.year = y;

  // Check for dash
  read_filler(text, pos, '-');

  // Read month
  int m = read_part(text, pos, 2);
  if (m < 0) return;
  split.month = m;

  // Check for dash
  read_filler(text, pos, '-');

  // Read day
  int d = read_part(text, pos, 2);
  if (d < 0) return;
  split.day = d;

  // Check for space or 'T', otherwise fail if not lenient
  if (!read_filler(text, pos, ' ') && !read_filler(text, pos, 'T') &&
      !lenient)
    return;

  // Read hour
  int h = read_part(text, pos, 2);
  if (h == OT_BAD_DATA || (h == OT_NO_DATA && !lenient)) return;
  if (h >= 0)
  {
    split.hour = h;

    // Check for colon
    read_filler(text, pos, ':');

    // Read minute
    int mi = read_part(text, pos, 2);
    if (mi == OT_BAD_DATA || (h == OT_NO_DATA && !lenient)) return;
    if (mi >= 0)
    {
      split.min = mi;

      // Check for colon
      read_filler(text, pos, ':');

      // Read seconds as float for all the rest
      double s = read_part_f(text, pos, text.size() - pos);
      if (s == OT_BAD_DATA || (s == OT_NO_DATA && !lenient)) return;
      if (s >= 0) split.sec = s;
    }
  }

  // Set timestamp
  t = combine(split);
}

//------------------------------------------------------------------------
// Convert to ISO timestamp string
// Generates YYYY:MM:DDTHH:MM:SS.sssZ form or empty if invalid
// This format is also compatible with XML
string Stamp::iso() const
{
  if (!t) return "";  // Empty if invalid

  ostringstream oss;
  Split sp = split(t);  

  // Fudge to pad floating seconds, because setw() won't do it
  const char *pad = "";
  if (sp.sec < 10) pad = "0";

  // Fix precision for small values
  int prec = 11;
  if (sp.sec < 10) prec--;
  if (sp.sec < 1) prec--;

  oss << setw(2) << setfill('0') << sp.year  << '-'
      << setw(2) << setfill('0') << sp.month << '-'
      << setw(2) << setfill('0') << sp.day   << 'T'
      << setw(2) << setfill('0') << sp.hour  << ':'
      << setw(2) << setfill('0') << sp.min   << ':'
      << pad << setprecision(prec) << sp.sec << "Z";

  return oss.str();
}

//------------------------------------------------------------------------
// Convert to ISO date
// Generates YYYY-MM-DD form or empty if invalid
// sep can be specified, defaults to '-' - set to 0 to leave out
// This format is also compatible with XML
string Stamp::iso_date(char sep) const
{
  if (!t) return "";  // Empty if invalid

  ostringstream oss;
  Split sp = split(t);  

  oss << setw(2) << setfill('0') << sp.year;
  if (sep) oss << sep;
  oss << setw(2) << setfill('0') << sp.month;
  if (sep) oss << sep;
  oss << setw(2) << setfill('0') << sp.day;

  return oss.str();
}

//------------------------------------------------------------------------
// Convert to HH:MM with optional (:SS)
// sep can be specified, defaults to ':' - set to 0 to leave out
// Generates HH:MM:SS form or empty if invalid
string Stamp::iso_time(char sep, bool with_secs) const
{
  if (!t) return "";  // Empty if invalid

  ostringstream oss;
  Split sp = split(t);  

  oss << setw(2) << setfill('0') << sp.hour;
  if (sep) oss << sep;
  oss << setw(2) << setfill('0') << sp.min;

  if (with_secs && sep) oss << sep;
  if (with_secs) oss << setw(2) << setfill('0') << (int)sp.sec;

  return oss.str();
}

//------------------------------------------------------------------------
// Get the locale-specific date format
string Stamp::locale_date() const
{
  return format("%x");
}

//------------------------------------------------------------------------
// Get the locale-specific time format
string Stamp::locale_time() const
{
  return format("%X");
}

//------------------------------------------------------------------------
// Get the locale-specific date and time format
string Stamp::locale_date_time() const
{
  return format("%x %H:%M");
}

//------------------------------------------------------------------------
// Format according to strftime format (max 40 chars)
string Stamp::format(const char *format) const
{
  struct tm tm;
  get_tm(tm);
  char buf[40];
  size_t len = strftime(buf, 40, format, &tm);  
  return string(buf, len);
}

//------------------------------------------------------------------------
// Get the day of the week (Monday=1, Sunday=7)
int Stamp::weekday() const
{
  // Get days since epoch
  unsigned long seconds = (unsigned long)(t>>NTP_SHIFT); 
  unsigned long days = seconds/3600/24;

  // 1st January 1900 was a Monday, so..
  return 1+(days % 7);
}

//------------------------------------------------------------------------
// Get a timestamp in local time (according to TZ) from a normal UTC one
Stamp Stamp::localise() const
{
  // No other way to do this than localtime_r, without understanding the
  // entire complexity of TZ, DST, databases etc.
  time_t t = time();
  struct tm tm;
#if defined(__WIN32__)
  // Windows doesn't have localtime_r, but localtime is supposed to use
  // a thread-local buffer, so it's OK to use it directly
  tm = *localtime(&t);
#else
  // Use reentrant version
  localtime_r(&t, &tm);
#endif

  // Rebuild our split from this, pretending it's a GM time
  Split sp;
  sp.year  = tm.tm_year+1900;
  sp.month = tm.tm_mon+1; // We store 1..12, tm_mon is 0..11
  sp.day   = tm.tm_mday;
  sp.hour  = tm.tm_hour;
  sp.min   = tm.tm_min;
  sp.sec   = tm.tm_sec;

  return Stamp(sp);
}

//------------------------------------------------------------------------
// Fill a struct tm 
void Stamp::get_tm(struct tm& tm) const
{
  // Split it
  Split sp = split(t);
  
  // Fill tm
  tm.tm_year = sp.year-1900;
  tm.tm_mon  = sp.month-1;  // They want 0.11
  tm.tm_mday = sp.day;
  tm.tm_hour = sp.hour;
  tm.tm_min  = sp.min;
  tm.tm_sec  = (int)sp.sec;
  tm.tm_wday = weekday() % 7;  // Move Sunday=7 back to Sunday=0
}

//------------------------------------------------------------------------
// Get a timestamp in UTC time from a localised one
Stamp Stamp::globalise() const
{
  struct tm tm;
  get_tm(tm);

  // Get new time from this (unlocalises it)
  return Stamp(mktime(&tm));
}

//------------------------------------------------------------------------
// Static constructor-like function for time now
Stamp Stamp::now()
{
  Stamp s;
#if defined(__WIN32__)
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);

  // Get get 64-bit FILETIME (100ns since 1-1-1601)
  uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) + ft.dwLowDateTime; 

  // Split into seconds and microseconds
  uint64_t secs = t/(10*MICRO);  // 10*(100ns) = 1us
  uint64_t usecs = (t/10)-secs*MICRO;

  // Convert Epochs - first to time_t (1970), then to NTP (1900)
  secs -= FILETIME_UNIX_EPOCH_DIFF;
  secs += EPOCH_1970;

  // Now form NTP shifted version
  s.t = (secs<<NTP_SHIFT) + (usecs<<NTP_SHIFT)/MICRO;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);

  s.t = (ntp_stamp_t)(tv.tv_sec + EPOCH_1970) << NTP_SHIFT;
  s.t += ((ntp_stamp_t)tv.tv_usec << NTP_SHIFT)/MICRO;
#endif

  return s;
}

//------------------------------------------------------------------------
// << operator to write Stamp to ostream
ostream& operator<<(ostream& s, const Stamp& st)
{
  s << st.iso();
  return s;
}

}} // namespaces
