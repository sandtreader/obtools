# CLAUDE.md - ObTools::Time Library

## Overview

`ObTools::Time` provides timestamp, duration, and date handling with extensive formatting and parsing. Internal representation is 64-bit seconds from 1900-01-01 with 20-bit fixed-point fraction. Lives under `namespace ObTools::Time`.

**Header:** `ot-time.h`
**Dependencies:** `ot-text`

## Constants

```cpp
MINUTE=60, HOUR=3600, DAY=86400, WEEK=604800
MILLI=1000, MICRO=1000000, NANO=1000000000
```

## Key Classes

| Class | Purpose |
|-------|---------|
| `Split` | Broken-down time (year/month/day/hour/min/sec) |
| `Duration` | Length of time (double seconds) |
| `Stamp` | Absolute moment in UTC (64-bit fixed-point) |
| `DateStamp` | Date-only stamp (midnight, inherits Stamp) |
| `DateInterval` | Calendar interval (days/weeks/months/years) |

## Split

```cpp
Split();                                              // 1900-01-01 00:00:00
Split(int year, int month, int day, int hour=0, int min=0, int sec=0);
void normalise();                                      // fix out-of-range
bool operator==(const Split& o) const;
```

## Duration

```cpp
Duration();  explicit Duration(double seconds);
Duration(const string& text);
// Accepts: "30", "1:30", "2:30:00", "5 min", "1 hour 30 min",
//          "P1DT12H30M", "PT5S" (ISO 8601)

double seconds() const;  uint64_t milliseconds() const;
string hms() const;     // "HH:MM:SS"
string unit() const;    // "1 hour 30 min"
string iso() const;     // "P1DT12H30M"

bool valid() const;  explicit operator bool() const;
bool is_negative() const;  Duration abs() const;
static Duration clock();   // monotonic clock

// Arithmetic: +, -, *, /, +=, -=, *=, /=, unary -
// Comparison: ==, !=, <, >, <=, >=
```

## Stamp

```cpp
Stamp();  Stamp(time_t t);
Stamp(const Split& split);
Stamp(const string& text, bool lenient=false);
// Accepts: ISO8601, RFC822, RFC850, asctime
// lenient=true allows time parts to be omitted

static Stamp now();
static Stamp from_ntp(ntp_stamp_t n);
static Stamp from_jdn(double j);

bool valid() const;  explicit operator bool() const;
time_t time() const;          // UNIX epoch
ntp_stamp_t ntp() const;      // NTP 64-bit
double jdn() const;           // Julian Day Number

// Formatting
string iso() const;            // "2024-01-15T10:30:00.000Z"
string iso_minimal() const;   // "20240115T103000"
string iso_numeric() const;   // "20240115103000"
string iso_date(char sep='-') const;   // "2024-01-15"
string iso_time(char sep=':', bool with_secs=false) const;
string rfc822() const;        // "Mon, 15-Jan-2024 10:30:00 GMT"
string sql() const;
string format(const char *fmt) const;  // strftime

Split split() const;
Stamp date() const;            // midnight
int weekday() const;           // Monday=1, Sunday=7
Stamp localise() const;       // UTC -> local
Stamp globalise() const;      // local -> UTC

// Arithmetic
Duration operator-(const Stamp& o) const;    // difference
Stamp operator+(const Duration& d) const;    // add duration
Stamp operator-(const Duration& d) const;
Stamp operator+(double seconds) const;
// +=, -= variants
// Comparison: ==, !=, <, >, <=, >=
```

## DateStamp

Inherits `Stamp`, rounded to midnight. Displays date only.

```cpp
DateStamp();  DateStamp(time_t t);  DateStamp(const Stamp& stamp);
DateStamp(const string& text);     // "YYYY-MM-DD" or "YYYYMMDD"

string iso() const;               // "2024-01-15"
string iso_datetime() const;     // full Stamp::iso()
int jdn() const;                  // Julian Day Number (integer)

DateStamp operator+(const DateInterval& di) const;
DateStamp operator-(const DateInterval& di) const;
// Also inherits Duration arithmetic from Stamp
```

## DateInterval

Calendar intervals (months/years are variable-length, so Duration can't represent them).

```cpp
DateInterval();
DateInterval(int number, Unit unit);       // Unit: days/weeks/months/years
DateInterval(int number, const string& unit_str);
DateInterval(const string& str);           // "3 months", "1 year"

bool operator!() const;
string str() const;
void add_to(Split& sp) const;
void subtract_from(Split& sp) const;
```

## File Layout

```
ot-time.h             - Public header
stamp.cc              - Stamp implementation
date-stamp.cc         - DateStamp
duration.cc           - Duration
date-interval.cc      - DateInterval
split.cc              - Split
test-stamp.cc         - Stamp tests (gtest)
test-duration.cc      - Duration tests
test-date-interval.cc - DateInterval tests
test-parse.cc         - Parsing tests
test-split.cc         - Split tests
test-exhaustive-dates.cc - Exhaustive date tests
```
