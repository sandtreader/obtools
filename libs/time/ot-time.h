//==========================================================================
// ObTools::Time: ot-time.h
//
// Public definitions for ObTools::Time
// Representation and conversion of timestamps and durations
// 
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_TIME_H
#define __OBTOOLS_TIME_H

#include <string>
#include <map>
#include <ctime>
#include <stdint.h>

namespace ObTools { namespace Time { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Useful constants

// Multiples of seconds
const unsigned int MINUTE = 60;
const unsigned int HOUR   = 3600;
const unsigned int DAY    = 24*HOUR;
const unsigned int WEEK   = 7*DAY;

// Fractions of seconds
const unsigned int MILLI = 1000;
const unsigned int MICRO = 1000000;
const unsigned int NANO  = 1000000000;

// Conversion constants
const unsigned long EPOCH_1970 = 2208988800UL;  // 1970-1900

// NTP format
const unsigned int NTP_SHIFT = 32;
typedef uint64_t ntp_stamp_t;

//==========================================================================
//Split - a split-up time structure, similar to 'struct tm'
struct Split
{
  int year;   // Full year - e.g. 2005
  int month;  // 1-12
  int day;    // 1-31
  int hour;   // 0-23
  int min;    // 0-59
  double sec; // 0-59.999999

  Split():
    year(1900), month(1), day(1), hour(0), min(0), sec(0) {}
};

//==========================================================================
//Duration - a length of time, not fixed at any one moment
//Beware: Falls apart if durations are negative!
class Duration
{
private:
  double t;      // Seconds

  // Unit dictionary - just one static instance of this
  static struct UnitDictionary
  {
    map<string, int> multiples; 
    map<string, int> fractions; 

    UnitDictionary();  // Static initialiser in duration.cc
  } units;

public:
  //------------------------------------------------------------------------
  // Default constructor
  Duration(): t(0.0) {}

  //------------------------------------------------------------------------
  // Constructor from double
  Duration(double _t): t(_t) {}

  //------------------------------------------------------------------------
  // Constructor from string
  // Accepts any of the following forms:
  //  1234       seconds
  //  1234.2323  seconds & fractional seconds
  //  33:20.23   minutes & seconds
  //  2:33:20    hours, minutes & seconds
  //  10:0:0:0   days, hours, minutes & seconds
  //  1 xxx      Number of units.  Number may be decimal - units are:
  //               ns - nanoseconds
  //               us - microseconds
  //               ms - milliseconds
  //               cs - centiseconds 
  //               s(ec(ond)(s)) - seconds
  //               min(ute)(s)   - minutes
  //               h(our(s))     - hours
  //               d(ay(s))      - calendar days
  //               w(eek(s))     - weeks
  //
  //             Note:  months and years are NOT accepted, because their
  //             duration depends on when you ask
  Duration(const string& text);

  //------------------------------------------------------------------------
  // Static constructor-like function from NTP timestamp
  // (done this way to avoid ambiguity with time_t version)
  static Duration from_ntp(ntp_stamp_t n)
  { return Duration((double)n/(1ULL<<NTP_SHIFT)); }

  //------------------------------------------------------------------------
  // Validity checks - 0 time is not valid
  bool valid() const { return t!=0; }
  bool operator!() { return !t; }

  //------------------------------------------------------------------------
  // Convert to floating point seconds (e.g. for NTP timestamp in text)
  double seconds() const { return t; }

  //------------------------------------------------------------------------
  // Convert to NTP timestamp - whole 64 bits, fixed point at 32
  ntp_stamp_t ntp() const { return (uint64_t)(t * (1ULL<<NTP_SHIFT)); }

  //------------------------------------------------------------------------
  // Convert to HH:MM:SS string - never goes into days or higher
  // Can output fractional seconds - rounds to nearest millisecond
  string hms() const;

  //------------------------------------------------------------------------
  // Arithmetic operators, so far as it makes sense
  Duration operator-(const Duration& o) const { return Duration(t-o.t); }
  Duration operator+(const Duration& o) const { return Duration(t+o.t); }
  Duration operator*(double n) const { return Duration(t*n); }
  Duration operator/(double n) const { return Duration(t/n); }

  //------------------------------------------------------------------------
  // Comparison operators
  bool operator==(const Duration& o) const { return t==o.t; }
  bool operator!=(const Duration& o) const { return t!=o.t; }
  bool operator<(const Duration& o) const { return t<o.t; }
  bool operator>(const Duration& o) const { return t>o.t; }
  bool operator<=(const Duration& o) const { return t<=o.t; }
  bool operator>=(const Duration& o) const { return t>=o.t; }

  //------------------------------------------------------------------------
  // Constructor-like static function to return monotonic clock - baseline
  // unknown, but guaranteed never to get mangled by ntpd, DST et al.
  // Returns Duration(0.0) if clock not available
  static Duration clock();
};

//==========================================================================
//Stamp - fixed moment in absolute time
//Timestamps are stored in GMT (UTC, Z), and converted from local time on
//creation.
//Internal format 64-bit NTP timestamp
class Stamp
{
protected:
  ntp_stamp_t t;
  static ntp_stamp_t combine(const Split& split);
  static Split split(ntp_stamp_t ts);
  void get_tm(struct tm& tm) const;

public:
  //------------------------------------------------------------------------
  // Default constructor - 0
  Stamp(): t(0) {}

  //------------------------------------------------------------------------
  // Constructor from time_t
  Stamp(time_t _t): t((ntp_stamp_t)(_t+EPOCH_1970)<<NTP_SHIFT) {}

  //------------------------------------------------------------------------
  // Constructor from split time
  Stamp(const Split& split): t(combine(split)) {}

  //------------------------------------------------------------------------
  // Constructor from string
  // Reads one of the following formats:
  //   ISO8601:  YYYY[-]MM[-]DD(T| )HH[:]MM[:]SS.ss(Z|+00) (UTC always assumed)
  //   HTTP:     to come!
  //   RFC822:   to come!
  Stamp(const string& text);

  //------------------------------------------------------------------------
  // Validity checks - NTP 0 is not valid
  bool valid() const { return t!=0; }
  bool operator!() { return !t; }

  //------------------------------------------------------------------------
  // Convert to time_t
  time_t time() const { return (time_t)(t>>NTP_SHIFT)-EPOCH_1970; }

  //------------------------------------------------------------------------
  // Get more accurate partial seconds count (0-59.999')
  double seconds() const 
  { ntp_stamp_t s = t%(60ULL<<NTP_SHIFT);  // Partial seconds
    return (double)s/(1ULL<<NTP_SHIFT); }

  //------------------------------------------------------------------------
  // Convert to NTP timestamp - whole 64 bits, fixed point at 32
  ntp_stamp_t ntp() const { return t; } 

  //------------------------------------------------------------------------
  // Convert to ISO timestamp string
  // Generates YYYY-MM-DDTHH:MM:SS.sssZ form or empty if invalid
  // This format is also compatible with XML
  string iso() const;

  //------------------------------------------------------------------------
  // Convert to ISO date
  // Generates YYYY-MM-DD form or empty if invalid
  // This format is also compatible with XML
  string iso_date() const;

  //------------------------------------------------------------------------
  // Get the locale-specific date format
  string locale_date() const;

  //------------------------------------------------------------------------
  // Get the locale-specific time format
  string locale_time() const;

  //------------------------------------------------------------------------
  // Get the locale-specific date and time format
  string locale_date_time() const;
    
  //------------------------------------------------------------------------
  // Split the timestamp into individual items
  void split(Split &sp) const { sp = split(t); }

  //------------------------------------------------------------------------
  // Alternate form, returning it
  Split split() const { return split(t); }

  //------------------------------------------------------------------------
  // Get the day of the week (Monday=1, Sunday=7)
  int weekday() const;

  //------------------------------------------------------------------------
  // Get a timestamp in local time (according to TZ) from a normal UTC one
  // Note: Fractional seconds are lost
  Stamp localise() const; 

  //------------------------------------------------------------------------
  // Get a timestamp in UTC time from a localised one
  // Note: Fractional seconds are lost
  Stamp globalise() const;

  //------------------------------------------------------------------------
  // Static constructor-like function from NTP timestamp
  // (done this way to avoid ambiguity with time_t version)
  static Stamp from_ntp(ntp_stamp_t n) { Stamp s; s.t = n; return s; }

  //------------------------------------------------------------------------
  // Static constructor-like function for time now
  static Stamp now();

  //------------------------------------------------------------------------
  // Subtract two stamps to get duration between
  // Beware if they could be misordered - duration returned will be very
  // large, not negative!
  Duration operator-(const Stamp& o) const
  { return Duration::from_ntp(t-o.t); }

  //------------------------------------------------------------------------
  // Add a Duration to a stamp
  Stamp operator+(const Duration& d) const
  { return Stamp::from_ntp(t+d.ntp()); }

  Stamp& operator+=(const Duration& d)
  { t += d.ntp(); return *this; }

  //------------------------------------------------------------------------
  // Subtract a Duration from a stamp
  Stamp operator-(const Duration& d) const
  { return Stamp::from_ntp(t-d.ntp()); }

  Stamp& operator-=(const Duration& d)
  { t -= d.ntp(); return *this; }

  //------------------------------------------------------------------------
  // Comparison operators
  // Think very hard before you use exact inequality/equality - two
  // different timestamps are very unlikely to be the same...  Provided
  // really only for piecewise comparison of structures that contain them
  bool operator==(const Stamp& o) const { return t==o.t; }
  bool operator!=(const Stamp& o) const { return t!=o.t; }

  // These are much safer...
  bool operator<(const Stamp& o) const { return t<o.t; }
  bool operator>(const Stamp& o) const { return t>o.t; }
  bool operator<=(const Stamp& o) const { return t<=o.t; }
  bool operator>=(const Stamp& o) const { return t>=o.t; }
};

//==========================================================================
//DateStamp - fixed date in absolute time
//As per Stamp, but can be constructed from just a date
class DateStamp: public Stamp
{
public:
  // Default constructor
  DateStamp() {}

  //------------------------------------------------------------------------
  // Constructor from time_t
  DateStamp(time_t _t): Stamp(_t) {}

  //------------------------------------------------------------------------
  // Constructor from split time
  DateStamp(const Split& split): Stamp(split) {}

  //------------------------------------------------------------------------
  // Constructor from Stamp
  DateStamp(const Stamp& stamp): Stamp(stamp) {}

  //------------------------------------------------------------------------
  // Constructor from string
  // Reads the following formats:
  //   ISO8601:  YYYY[-]MM[-]DD (UTC always assumed)
  DateStamp(const string& text);

  //------------------------------------------------------------------------
  // Convert to ISO date string
  string iso() const
  {
    return Stamp::iso_date();
  }

  //------------------------------------------------------------------------
  // Convert to ISO datetime string
  string iso_datetime() const
  {
    return Stamp::iso();
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TIME_H
















