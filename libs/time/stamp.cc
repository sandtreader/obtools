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
#include <vector>
#include <ot-text.h>

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

//--------------------------------------------------------------------------
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
  ntp_stamp_t ts = static_cast<ntp_stamp_t>(seconds) << NTP_SHIFT;
  ts += static_cast<ntp_stamp_t>(split.sec * (1ULL << NTP_SHIFT));

  return ts;
}

//--------------------------------------------------------------------------
// Helper function to split an internal timestamp into a split time
Split Stamp::split(ntp_stamp_t ts)
{
  // For this we could use gmtime, in theory, but it's not threadsafe!
  // So, we're back to doing it ourselves...
  Split sp;

  // First we downgrade to integer to make life easier - we'll add
  // back the fractional part later.
  unsigned long seconds = static_cast<unsigned long>(ts>>NTP_SHIFT);

  // Get estimate of years - near enough for NTP validity timeframe
  int years = static_cast<int>(seconds/(365.24*DAY));
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
  sp.sec += static_cast<double>(ts & ((1ULL<<NTP_SHIFT)-1))/(1ULL<<NTP_SHIFT);

  return sp;
}

//--------------------------------------------------------------------------
// Constructor from string
// See ot-time.h for details

// Helpers
namespace
{
  const int OT_NO_DATA = -1;  // Avoid Windows NO_DATA macro! :-(
  const int OT_BAD_DATA = -2;

  // Read an integer field in length characters, modifying pos
  int read_part(const string& text, string::size_type size,
                string::size_type& pos, int length)
  {
    if (pos == size)
      return OT_NO_DATA;

    if (pos + length > size)
      return OT_BAD_DATA;

    string s(text, pos, length);
    for (string::const_iterator it = s.begin(); it != s.end(); ++it)
      if (*it < '0' || *it > '9')
        return OT_BAD_DATA;

    pos += length;
    return atoi(s.c_str());
  }

  // Read a floating field in length characters, modifying pos
  double read_part_f(const string& text, string::size_type size,
                     string::size_type& pos, int length)
  {
    if (pos == size)
      return OT_NO_DATA;

    if (length < 2)
      return OT_BAD_DATA;

    if (pos + length > size)
      return OT_BAD_DATA;

    string s(text, pos, length);
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

  // Read an optional filler character
  bool read_filler(const string& text, string::size_type size,
                   string::size_type& pos, char c)
  {
    if (pos < size)
    {
      if (text[pos] == c)
      {
        pos++;
        return true;
      }
    }
    return false;
  }

  // Read HH[:]MM[:]SS into time portion of a split, returns whether successful
  // If float_secs is set, accepts floating point seconds
  bool read_time(const string& text, string::size_type size,
                 string::size_type& pos, bool lenient,
                 bool float_secs, Split& split)
  {
    // Read hour
    int h = read_part(text, size, pos, 2);
    if (h == OT_BAD_DATA || (h == OT_NO_DATA && !lenient)) return false;
    if (h >= 0)
    {
      split.hour = h;

      // Check for colon
      read_filler(text, size, pos, ':');

      // Read minute
      int mi = read_part(text, size, pos, 2);
      if (mi == OT_BAD_DATA || (h == OT_NO_DATA && !lenient)) return false;
      if (mi >= 0)
      {
        split.min = mi;

        // Check for colon
        read_filler(text, size, pos, ':');

        // Read seconds as float for all the rest
        double s = float_secs?read_part_f(text, size, pos, size - pos)
                             :read_part(text, size, pos, 2);
        if (s == OT_BAD_DATA || (s == OT_NO_DATA && !lenient)) return false;
        if (s >= 0) split.sec = s;
      }
    }

    return true;
  }

  // Read a timezone offset {+|-}[hh[[:]mm]]
  bool read_timezone_offset(const string& text, string::size_type size,
                            string::size_type pos, Duration& tz_offset)
  {
    int sign = 1;
    if (pos >= size) return false;
    if (text[pos++] == '-') sign = -1;

    // Get hour
    int h = read_part(text, size, pos, 2);
    if (h<0) return false;

    // Check for optional colon
    bool colon = read_filler(text, size, pos, ':');

    // Look for minutes - must be there if colon present
    int m = read_part(text, size, pos, 2);
    if (m<0)
    {
      if (colon) return false;
      m=0;
    }

    tz_offset = Duration(sign*(h*3600.0+m*60));
    return pos == size;  // Must be all used up
  }

  // Read an ISO timestamp into split, returns whether successful
  // Sets timezone offset of input text if any
  bool read_iso(const string& text, bool lenient, Split& split,
                Duration& tz_offset)
  {
    string::size_type pos=0;
    string::size_type size = text.size();

    // Read year
    int y = read_part(text, size, pos, 4);
    if (y < 0) return false;
    split.year = y;

    // Check for dash
    read_filler(text, size, pos, '-');

    // Read month
    int m = read_part(text, size, pos, 2);
    if (m < 0) return false;
    split.month = m;

    // Check for dash
    read_filler(text, size, pos, '-');

    // Read day
    int d = read_part(text, size, pos, 2);
    if (d < 0) return false;
    split.day = d;

    // Check for space or 'T', otherwise fail if not lenient
    if (!read_filler(text, size, pos, ' ') &&
        !read_filler(text, size, pos, 'T') &&
        !lenient)
      return false;

    // Look for timezone - we accept Z or [hh[[:]mm]
    tz_offset = Duration(0);
    for(string::size_type p = pos; p<size; p++)
    {
      switch (text[p])
      {
        case '-':
        case '+':
          if (!read_timezone_offset(text, size, p, tz_offset))
            return false;

          // Falling!

        case 'Z':    // Offset is still 0
          size = p;  // Truncate time and stop
          break;

        default:;
      }
    }

    // Read ISO time
    if (!read_time(text, size, pos, lenient, true, split)) return false;

    // Make sure we've read everything
    return (pos == size);
  }

  // Get a month number (1-12) from a 3-character word, or 0 if invalid
  int get_month(const string& word)
  {
         if (word == "Jan") return 1;
    else if (word == "Feb") return 2;
    else if (word == "Mar") return 3;
    else if (word == "Apr") return 4;
    else if (word == "May") return 5;
    else if (word == "Jun") return 6;
    else if (word == "Jul") return 7;
    else if (word == "Aug") return 8;
    else if (word == "Sep") return 9;
    else if (word == "Oct") return 10;
    else if (word == "Nov") return 11;
    else if (word == "Dec") return 12;
    else return 0;
  }

  // Read RFC822/RFC1123 into split, returns whether successful
  //    Sun, 06 Nov 1994 08:49:37 GMT
  bool read_rfc_822(vector<string>& words, Split& split)
  {
    // Ignore weekday

    // Day
    split.day = Text::stoi(words[1]);
    if (!split.day) return false;

    // Month
    split.month = get_month(words[2]);
    if (!split.month) return false;

    // Year
    split.year = Text::stoi(words[3]);
    if (!split.year) return false;

    // Read time
    string::size_type pos=0;
    if (!read_time(words[4], words[4].size(), pos, false, false, split))
      return false;

    // Last word must be GMT
    // ! Check for time zone modifier?
    return (words[5] == "GMT");
  }

  // Read RFC850/RFC1036 into split, returns whether successful
  //    Sunday, 06-Nov-94 08:49:37 GMT
  bool read_rfc_850(vector<string>& words, Split& split)
  {
    // Ignore weekday

    // Split date into 3 bits
    vector<string> bits = Text::split(words[1], '-', false, 3);
    if (bits.size() != 3) return false;

    // Day
    split.day = Text::stoi(bits[0]);
    if (!split.day) return false;

    // Month
    split.month = get_month(bits[1]);
    if (!split.month) return false;

    // Year
    split.year = Text::stoi(bits[2]);
    if (!split.year) return false;
    // Arbitary split to guess century - based on the fact that NTP
    // (which is our core storage format) can't go beyond 2036 anyway
    if (split.year < 37)
      split.year += 2000;
    else
      split.year += 1900;

    // Read time
    string::size_type pos=0;
    if (!read_time(words[2], words[2].size(), pos, false, false, split))
      return false;

    // Last word must be GMT
    // ! Check for time zone modifier?
    return (words[3] == "GMT");
  }

  // Read asctime() format into split, returns whether successful
  //    Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
  bool read_asctime(vector<string>& words, Split& split)
  {
    // Ignore weekday

    // Month
    split.month = get_month(words[1]);
    if (!split.month) return false;

    // Day
    split.day = Text::stoi(words[2]);
    if (!split.day) return false;

    // Read time
    string::size_type pos=0;
    if (!read_time(words[3], words[3].size(), pos, false, false, split))
      return false;

    // Year
    split.year = Text::stoi(words[4]);
    if (!split.year) return false;

    return true;
  }

  // Read an HTTP textual timestamp into split, returns whether successful
  // Handles RFC822, RFC 850, asctime - e.g. (from RFC2616#3.3.1)
  //    Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
  //    Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
  //    Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
  // Never lenient in the sense of missing off time parts
  bool read_http(const string& text, Split& split)
  {
    vector<string> words = Text::split_words(text);
    switch (words.size())
    {
      case 6:  // RFC 822
        return read_rfc_822(words, split);

      case 4:  // RFC 850
        return read_rfc_850(words, split);

      case 5:  // asctime
        return read_asctime(words, split);

      default:
        return false;
    }
  }
}

Stamp::Stamp(const string& text, bool lenient)
{
  t=0; // Clear stamp incase we fail
  Split split;
  Duration tz_offset;

  // If the first part is a digit, assume it's ISO
  if (isdigit(text[0]))
  {
    if (!read_iso(text, lenient, split, tz_offset)) return;
  }
  else // assume it's an HTTP text format
  {
    if (!read_http(text, split)) return;
  }

  // Combined to NTP timestamp
  t = combine(split);

  // Modify for timezone - positive timezones are subtracted
  *this -= tz_offset;
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// Convert to minimal (and URL-safe) ISO-compatible timestamp string
// Generates YYYYMMDDTHHMMSS form or empty if invalid
string Stamp::iso_minimal() const
{
  if (!t) return "";  // Empty if invalid

  ostringstream oss;
  Split sp = split(t);

  oss << setw(2) << setfill('0') << sp.year
      << setw(2) << setfill('0') << sp.month
      << setw(2) << setfill('0') << sp.day   << 'T'
      << setw(2) << setfill('0') << sp.hour
      << setw(2) << setfill('0') << sp.min
      << setw(2) << setfill('0') << static_cast<int>(sp.sec);

  return oss.str();
}

//--------------------------------------------------------------------------
// Convert to numeric-only ISO-compatible timestamp string
// May not be considered valid by all parsers - including ours if non-lenient
// Generates YYYYMMDDHHMMSS form or empty if invalid
string Stamp::iso_numeric() const
{
  if (!t) return "";  // Empty if invalid

  ostringstream oss;
  Split sp = split(t);

  oss << setw(2) << setfill('0') << sp.year
      << setw(2) << setfill('0') << sp.month
      << setw(2) << setfill('0') << sp.day
      << setw(2) << setfill('0') << sp.hour
      << setw(2) << setfill('0') << sp.min
      << setw(2) << setfill('0') << static_cast<int>(sp.sec);

  return oss.str();
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
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
  if (with_secs) oss << setw(2) << setfill('0') << static_cast<int>(sp.sec);

  return oss.str();
}

//--------------------------------------------------------------------------
// Get the locale-specific date format
string Stamp::locale_date() const
{
  return format("%x");
}

//--------------------------------------------------------------------------
// Get the locale-specific time format
string Stamp::locale_time() const
{
  return format("%X");
}

//--------------------------------------------------------------------------
// Get the locale-specific date and time format
string Stamp::locale_date_time() const
{
  return format("%x %H:%M");
}

//--------------------------------------------------------------------------
// Convert to RFC822 string
// Generates Wdy, DD-Mon-YYYY HH:MM:SS GMT, empty if invalid
string Stamp::rfc822() const
{
  if (!t) return "";  // Empty if invalid
  return format("%a, %d %b %Y %T GMT");
}

//--------------------------------------------------------------------------
// Format according to strftime format (max 40 chars)
string Stamp::format(const char *format) const
{
  struct tm tm;
  get_tm(tm);
  char buf[40];
  size_t len = strftime(buf, 40, format, &tm);
  return string(buf, len);
}

//--------------------------------------------------------------------------
// Get just the date part
Stamp Stamp::date() const
{
  auto sp = split();
  sp.hour = 0;
  sp.min = 0;
  sp.sec = 0;
  return Stamp{sp};
}

//--------------------------------------------------------------------------
// Get the day of the week (Monday=1, Sunday=7)
int Stamp::weekday() const
{
  // Get days since epoch
  unsigned long seconds = static_cast<unsigned long>(t>>NTP_SHIFT);
  unsigned long days = seconds/3600/24;

  // 1st January 1900 was a Monday, so..
  return 1+(days % 7);
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
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
  tm.tm_sec  = static_cast<int>(sp.sec);
  tm.tm_wday = weekday() % 7;  // Move Sunday=7 back to Sunday=0
}

//--------------------------------------------------------------------------
// Get a timestamp in UTC time from a localised one
Stamp Stamp::globalise() const
{
  struct tm tm;
  get_tm(tm);

  // Get new time from this (unlocalises it)
  return Stamp(mktime(&tm));
}

//--------------------------------------------------------------------------
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

  s.t = static_cast<ntp_stamp_t>(tv.tv_sec + EPOCH_1970) << NTP_SHIFT;
  s.t += (static_cast<ntp_stamp_t>(tv.tv_usec) << NTP_SHIFT)/MICRO;
#endif

  return s;
}

//--------------------------------------------------------------------------
// << operator to write Stamp to ostream
ostream& operator<<(ostream& s, const Stamp& st)
{
  s << st.iso();
  return s;
}

}} // namespaces
