//==========================================================================
// ObTools::Time: ot-time.h
//
// Public definitions for ObTools::Time
// Representation and conversion of timestamps and durations
//
// Copyright (c) 2005-2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_TIME_H
#define __OBTOOLS_TIME_H

#include <chrono>
#include <string>
#include <map>
#include <ctime>
#include <stdint.h>

namespace ObTools { namespace Time {

// Make our lives easier without polluting anyone else
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
const auto EPOCH_JDN = 2415021.5L; // JDN at 1 Jan 1900

// NTP format
const unsigned int NTP_SHIFT = 32;
typedef uint64_t ntp_stamp_t;

//==========================================================================
// Split - a split-up time structure, similar to 'struct tm'
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
// Duration - a length of time, not fixed at any one moment
// Beware: Falls apart if durations are negative!
class Duration
{
private:
  double t;      // Seconds

public:
  //------------------------------------------------------------------------
  // Default constructor
  Duration(): t(0.0) {}

  //------------------------------------------------------------------------
  // Constructor from double
  explicit Duration(double _t): t(_t) {}
  explicit Duration(int _t): t(_t) {}

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
  //               m(in(ute)(s)) - minutes
  //               h((ou)r(s))   - hours
  //               d(ay(s))      - calendar days
  //               w((ee)k(s))   - weeks
  //  1 x 2 y    Combined units
  //  P1DT12H30M ISO with days
  //  PT5S       ISO without days
  //             Note:  months and years are NOT accepted, because their
  //             duration depends on when you ask
  Duration(const string& text);

  //------------------------------------------------------------------------
  // Validity checks - 0 time is not valid
  bool valid() const { return t!=0; }
  bool operator!() const { return !t; }
  explicit operator bool() const { return t; }

  //------------------------------------------------------------------------
  // Check if negative
  bool is_negative() const { return t < 0; }

  //------------------------------------------------------------------------
  // Convert to floating point seconds (e.g. for NTP timestamp in text)
  double seconds() const { return t; }

  //------------------------------------------------------------------------
  // Convert to milliseconds
  uint64_t milliseconds() const { return t * 1000; }

  //------------------------------------------------------------------------
  // Convert to HH:MM:SS string - never goes into days or higher
  // Can output fractional seconds - rounds to nearest millisecond
  string hms() const;

  //------------------------------------------------------------------------
  // Convert to unit-based string - e.g. "1 hour", "3 min 4 sec"
  // Note, doesn't handle fractional seconds, only goes up to days.
  string unit() const;

  //------------------------------------------------------------------------
  // Convert to ISO duration string
  // Generates P[n]DT[n]H[n]M[n]S form or empty if invalid
  // This format is also compatible with XML
  // Note: Never generates months or years because they are variable
  string iso() const;

  //------------------------------------------------------------------------
  // Arithmetic operators, so far as it makes sense
  Duration operator-() const { return Duration(0.0-t); }
  Duration operator-(const Duration& o) const { return Duration(t-o.t); }
  Duration operator+(const Duration& o) const { return Duration(t+o.t); }
  Duration operator*(double n) const { return Duration(t*n); }
  Duration operator/(double n) const { return Duration(t/n); }
  double operator/(const Time::Duration& o) const { return t/o.t; }

  //------------------------------------------------------------------------
  // Arithmetic assignments
  Duration operator+=(const Duration& o) { t+=o.t; return *this; }
  Duration operator-=(const Duration& o) { t-=o.t; return *this; }
  Duration operator*=(double n) { t*=n; return *this; }
  Duration operator/=(double n) { t/=n; return *this; }

  //------------------------------------------------------------------------
  // Comparison operators
  bool operator==(const Duration& o) const { return t==o.t; }
  bool operator!=(const Duration& o) const { return t!=o.t; }
  bool operator<(const Duration& o) const { return t<o.t; }
  bool operator>(const Duration& o) const { return t>o.t; }
  bool operator<=(const Duration& o) const { return t<=o.t; }
  bool operator>=(const Duration& o) const { return t>=o.t; }

  //------------------------------------------------------------------------
  // Get absolute value
  Duration abs() const { return is_negative() ? -*this : *this; }

  //------------------------------------------------------------------------
  // Constructor-like static function to return monotonic clock - baseline
  // unknown, but guaranteed never to get mangled by ntpd, DST et al.
  // Returns Duration(0.0) if clock not available
  static Duration clock();
};

//--------------------------------------------------------------------------
// Arithmetic operators the other way
Duration operator*(double n, const Duration& d);

//==========================================================================
// Stamp - fixed moment in absolute time
// Timestamps are stored in GMT (UTC, Z), and converted from local time on
// creation.
// Internal format 64-bit NTP timestamp
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
  Stamp(time_t _t): t(static_cast<ntp_stamp_t>(_t+EPOCH_1970)<<NTP_SHIFT) {}

  //------------------------------------------------------------------------
  // Constructor from split time
  Stamp(const Split& split): t(combine(split)) {}

  //------------------------------------------------------------------------
  // Constructor from string
  // Reads one of the following formats:
  //   ISO8601:  YYYY[-]MM[-]DD(T| )HH[:]MM[:]SS.ss(Z|+00) (UTC always assumed)
  //   RFC822:   [day, ignored] DD MMM YYYY HH:MM:SS GMT
  //   RFC850:   [day+, ignored] DD-MMM-YY HH:MM:SS GMT (Y2K split at 1937)
  //   asctime:  [day ignored] MMM [D]D HH:MM:SS YYYY
  // The lenient flag allows the time parts to be omitted from the string
  // and zero value assumed for those parts (ISO only)
  Stamp(const string& text, bool lenient = false);

  //------------------------------------------------------------------------
  // Validity checks - NTP 0 is not valid
  bool valid() const { return t!=0; }
  bool operator!() const { return !t; }
  explicit operator bool() const { return t; }

  //------------------------------------------------------------------------
  // As a chrono::time_point
  operator chrono::high_resolution_clock::time_point() const
  {
    return chrono::high_resolution_clock::time_point{
      chrono::nanoseconds{(NANO * ((t >> NTP_SHIFT) - EPOCH_1970)) +
                          ((NANO * (t & 0xFFFFFFFF)) / 0x100000000)}};
  }

  chrono::high_resolution_clock::time_point time_point() const
  {
    return static_cast<chrono::high_resolution_clock::time_point>(*this);
  }

  //------------------------------------------------------------------------
  // Convert to time_t
  time_t time() const { return static_cast<time_t>(t>>NTP_SHIFT)-EPOCH_1970; }

  //------------------------------------------------------------------------
  // Get more accurate partial seconds count (0-59.999')
  double seconds() const
  { ntp_stamp_t s = t%(60ULL<<NTP_SHIFT);  // Partial seconds
    return static_cast<double>(s)/(1ULL<<NTP_SHIFT); }

  //------------------------------------------------------------------------
  // Convert to NTP timestamp - whole 64 bits, fixed point at 32
  ntp_stamp_t ntp() const { return t; }

  //------------------------------------------------------------------------
  // Convert to ISO timestamp string
  // Generates YYYY-MM-DDTHH:MM:SS.sssZ form or empty if invalid
  // This format is also compatible with XML
  string iso() const;

  //------------------------------------------------------------------------
  // Convert to minimal (and URL-safe) ISO-compatible timestamp string
  // Generates YYYYMMDDTHHMMSS form or empty if invalid
  string iso_minimal() const;

  //------------------------------------------------------------------------
  // Convert to numeric-only ISO-compatible timestamp string
  // May not be considered valid by all parsers - including ours if non-lenient
  // Generates YYYYMMDDHHMMSS form or empty if invalid
  string iso_numeric() const;

  //------------------------------------------------------------------------
  // Convert to ISO date
  // Generates YYYY-MM-DD form or empty if invalid
  // sep can be specified, defaults to '-' - set to 0 to leave out
  // This format is also compatible with XML
  string iso_date(char sep='-') const;

  //------------------------------------------------------------------------
  // Convert to HH:MM with optional (:SS)
  // sep can be specified, defaults to ':' - set to 0 to leave out
  // Generates HH:MM:SS form or empty if invalid
  string iso_time(char sep=':', bool with_secs=false) const;

  //------------------------------------------------------------------------
  // Convert to SQL format
  string sql() const;

  //------------------------------------------------------------------------
  // Format according to strftime format (max 40 chars)
  string format(const char *format) const;

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
  // Convert to RFC822 string
  // Generates Wdy, DD-Mon-YYYY HH:MM:SS GMT, empty if invalid
  string rfc822() const;

  //------------------------------------------------------------------------
  // Convert to Julian Days
  double jdn() const
  {
    return (t / 65536.0L / 65536.0L / DAY) + EPOCH_JDN;
  }

  //------------------------------------------------------------------------
  // Split the timestamp into individual items
  void split(Split &sp) const { sp = split(t); }

  //------------------------------------------------------------------------
  // Alternate form, returning it
  Split split() const { return split(t); }

  //------------------------------------------------------------------------
  // Get just the date part
  Stamp date() const;

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
  // Constructor from Julian day number
  static Stamp from_jdn(double j)
  {
    Stamp s;
    // Mulitply by 2^16 as double twice for good precision
    s.t = (j - EPOCH_JDN) * DAY * 65536.0L * 65536.0L;
    return s;
  }

  //------------------------------------------------------------------------
  // Static constructor-like function for time now
  static Stamp now();

  //------------------------------------------------------------------------
  // Subtract two stamps to get duration between
  // Beware if they could be misordered - duration returned will be very
  // large, not negative!
  Duration operator-(const Stamp& o) const
  { return Duration(ntp_to_seconds(t-o.t)); }

  //------------------------------------------------------------------------
  // Add a Duration to a stamp
  Stamp operator+(const Duration& d) const
  { return Stamp::from_ntp(t+seconds_to_ntp(d.seconds())); }

  Stamp& operator+=(const Duration& d)
  { t += seconds_to_ntp(d.seconds()); return *this; }

  Stamp operator+(double d) const
  { return operator+(Duration{d}); }

  Stamp& operator+=(double d)
  { return operator+=(Duration{d}); }

  //------------------------------------------------------------------------
  // Subtract a Duration from a stamp
  Stamp operator-(const Duration& d) const
  { return Stamp::from_ntp(t-seconds_to_ntp(d.seconds())); }

  Stamp& operator-=(const Duration& d)
  { t -= seconds_to_ntp(d.seconds()); return *this; }

  Stamp operator-(double d) const
  { return operator-(Duration{d}); }

  Stamp& operator-=(double d)
  { return operator-=(Duration{d}); }

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

  //------------------------------------------------------------------------
  // Helper functions to convert double seconds to/from NTP 64-bit
  // Note negative numbers are OK (this limits the range)
  static int64_t seconds_to_ntp(double s)
  {
    return static_cast<int64_t>(s * (1ULL<<NTP_SHIFT));
  }

  static double ntp_to_seconds(uint64_t n)
  {
    return static_cast<double>(n) / (1ULL<<NTP_SHIFT);
  }
};

//--------------------------------------------------------------------------
// << operator to write Stamp to ostream
ostream& operator<<(ostream& s, const Stamp& st);

//==========================================================================
// DateStamp - fixed date in absolute time
// As per Stamp, but rounded back to midnight, and displays just date
class DateStamp: public Stamp
{
  void fix_to_midnight() { t = date().ntp(); }

public:
  // Default constructor
  DateStamp() {}

  //------------------------------------------------------------------------
  // Constructor from time_t
  DateStamp(time_t _t): Stamp(_t) { fix_to_midnight(); }

  //------------------------------------------------------------------------
  // Constructor from split time
  DateStamp(const Split& split): Stamp(split) { fix_to_midnight(); }

  //------------------------------------------------------------------------
  // Constructor from Stamp
  DateStamp(const Stamp& stamp): Stamp(stamp) { fix_to_midnight(); }

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

//--------------------------------------------------------------------------
// << operator to write DateStamp to ostream
ostream& operator<<(ostream& s, const DateStamp& dst);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_TIME_H
















