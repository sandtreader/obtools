# ObTools::Time

A timestamp, duration, and date handling library for C++17 with extensive formatting and parsing support for ISO 8601, RFC 822, and other common formats.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Stamp**: absolute moments in UTC with sub-second precision (20-bit fraction)
- **Duration**: time lengths with flexible parsing ("5 min", "1:30", ISO 8601)
- **DateStamp**: date-only values with calendar arithmetic
- **DateInterval**: calendar intervals (months, years) for variable-length periods
- **Split**: broken-down time for component access
- **Parsing**: ISO 8601, RFC 822, RFC 850, asctime, natural units
- **Formatting**: ISO, RFC, SQL, strftime, locale-specific

## Dependencies

- `ot-text` - Text utilities

## Quick Start

```cpp
#include "ot-time.h"
using namespace ObTools;
```

### Current Time

```cpp
Time::Stamp now = Time::Stamp::now();
cout << now.iso() << endl;  // "2024-01-15T10:30:00.000Z"
```

### Parsing Timestamps

```cpp
// ISO 8601
Time::Stamp t1("2024-01-15T10:30:00Z");
Time::Stamp t2("20240115T103000");
Time::Stamp t3("2024-01-15 10:30:00");

// Lenient (time parts optional)
Time::Stamp t4("2024-01-15", true);         // midnight
Time::Stamp t5("2024-01-15T10", true);      // 10:00:00

// RFC 822
Time::Stamp t6("Mon, 15 Jan 2024 10:30:00 GMT");

// From UNIX time
Time::Stamp t7(time(nullptr));

// From NTP
Time::Stamp t8 = Time::Stamp::from_ntp(ntp_value);

// Julian Day Number
Time::Stamp t9 = Time::Stamp::from_jdn(2460323.5);
```

### Formatting

```cpp
Time::Stamp t = Time::Stamp::now();

string s1 = t.iso();            // "2024-01-15T10:30:00.000Z"
string s2 = t.iso_minimal();    // "20240115T103000"
string s3 = t.iso_numeric();    // "20240115103000"
string s4 = t.iso_date();       // "2024-01-15"
string s5 = t.iso_date(0);      // "20240115" (no separator)
string s6 = t.iso_time();       // "10:30"
string s7 = t.iso_time(':', true);  // "10:30:00"
string s8 = t.rfc822();         // "Mon, 15-Jan-2024 10:30:00 GMT"
string s9 = t.sql();            // SQL format
string s10 = t.format("%Y/%m/%d");  // "2024/01/15"
```

### Timestamp Components

```cpp
Time::Stamp t("2024-01-15T10:30:45Z");

Time::Split s = t.split();
int year  = s.year;    // 2024
int month = s.month;   // 1
int day   = s.day;     // 15
int hour  = s.hour;    // 10
int min   = s.min;     // 30
double sec = s.sec;    // 45.0

int dow = t.weekday();  // Monday=1, Sunday=7
Time::Stamp midnight = t.date();  // just the date part
```

### Duration

```cpp
// Create
Time::Duration d1(30.0);           // 30 seconds
Time::Duration d2("5 min");
Time::Duration d3("1 hour 30 min");
Time::Duration d4("2:30:00");       // 2h 30m
Time::Duration d5("PT1H30M");       // ISO 8601

// Query
double secs = d2.seconds();        // 300.0
uint64_t ms = d2.milliseconds();   // 300000

// Format
string hms = d3.hms();             // "01:30:00.000"
string unit = d3.unit();           // "1 hour 30 min"
string iso = d3.iso();             // "PT1H30M"

// Arithmetic
Time::Duration total = d2 + d3;
Time::Duration doubled = d2 * 2;
Time::Duration half = d2 / 2;
double ratio = d3 / d2;            // 18.0

// Comparison
if (d2 < d3) { /* true */ }

// Monotonic clock (for elapsed time measurement)
Time::Duration start = Time::Duration::clock();
// ... do work ...
Time::Duration elapsed = Time::Duration::clock() - start;
```

### Timestamp Arithmetic

```cpp
Time::Stamp now = Time::Stamp::now();
Time::Duration one_hour("1 hour");

Time::Stamp later = now + one_hour;
Time::Stamp earlier = now - one_hour;
Time::Stamp also_later = now + 3600.0;  // seconds

Time::Duration diff = later - now;      // Duration between stamps
```

### Timezone Conversion

```cpp
Time::Stamp utc = Time::Stamp::now();   // always UTC internally
Time::Stamp local = utc.localise();     // UTC -> local time
Time::Stamp back = local.globalise();   // local time -> UTC
```

### Date-Only Values

```cpp
// Create
Time::DateStamp d1("2024-01-15");
Time::DateStamp d2("20240115");
Time::DateStamp d3(Time::Stamp::now());   // rounds to midnight

// Format
string iso = d1.iso();  // "2024-01-15"
int jdn = d1.jdn();     // Julian Day Number

// Calendar arithmetic
Time::DateInterval month(1, Time::DateInterval::months);
Time::DateStamp next_month = d1 + month;

Time::DateInterval year(1, Time::DateInterval::years);
Time::DateStamp next_year = d1 + year;

// Duration arithmetic still works
Time::DateStamp tomorrow = d1 + Time::Duration("1 day");
```

### Date Intervals

```cpp
// Create
Time::DateInterval di1(3, Time::DateInterval::months);
Time::DateInterval di2(1, "year");
Time::DateInterval di3("6 months");

// Apply to dates
Time::DateStamp date("2024-01-31");
Time::DateStamp later = date + di1;   // adds 3 months

// Apply to split
Time::Split s = date.split();
di1.add_to(s);
di2.subtract_from(s);
s.normalise();

// Format
string str = di1.str();  // "3 months"
```

### Validity Checking

```cpp
Time::Stamp t;           // default: invalid (zero)
if (!t) { /* invalid */ }
if (t.valid()) { /* valid */ }

Time::Duration d;        // default: 0 (invalid for Duration)
if (!d) { /* zero duration */ }
```

## API Reference

### Stamp

| Method | Returns | Description |
|--------|---------|-------------|
| `now()` | `Stamp` | Current UTC time (static) |
| `iso()` | `string` | ISO 8601 with milliseconds |
| `iso_date()` | `string` | Date only |
| `rfc822()` | `string` | RFC 822 format |
| `split()` | `Split` | Broken-down components |
| `date()` | `Stamp` | Midnight of same day |
| `weekday()` | `int` | 1=Mon, 7=Sun |
| `time()` | `time_t` | UNIX epoch |
| `ntp()` | `ntp_stamp_t` | NTP format |
| `localise()` | `Stamp` | UTC to local |

### Duration

| Method | Returns | Description |
|--------|---------|-------------|
| `seconds()` | `double` | As floating-point seconds |
| `milliseconds()` | `uint64_t` | As milliseconds |
| `hms()` | `string` | "HH:MM:SS.mmm" |
| `unit()` | `string` | Natural units |
| `iso()` | `string` | ISO 8601 duration |
| `clock()` | `Duration` | Monotonic clock (static) |

### DateStamp

| Method | Returns | Description |
|--------|---------|-------------|
| `iso()` | `string` | "YYYY-MM-DD" |
| `jdn()` | `int` | Julian Day Number |
| `operator+/-` | `DateStamp` | Add/subtract DateInterval |

## Build

```
NAME    = ot-time
TYPE    = lib
DEPENDS = ot-text
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
