//==========================================================================
// ObTools::Time: test-format.cc
//
// Test harness for time library - output formatting methods
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"
#include <sstream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Stamp::iso() tests
TEST(FormatTest, TestISOOutputForKnownTimestamp)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  string iso = s.iso();
  EXPECT_EQ("2024-06-15T14:30:45Z", iso);
}

TEST(FormatTest, TestISOOutputForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.iso());
}

TEST(FormatTest, TestISOOutputForMidnightTimestamp)
{
  Time::Stamp s("2000-01-01T00:00:00Z");
  string iso = s.iso();
  EXPECT_NE("", iso);
  EXPECT_TRUE(iso.find("2000") != string::npos);
}

//--------------------------------------------------------------------------
// Stamp::iso_minimal() tests
TEST(FormatTest, TestISOMinimalOutput)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  string m = s.iso_minimal();
  // YYYYMMDDTHHMMSS
  EXPECT_EQ("20240615T143045", m);
}

TEST(FormatTest, TestISOMinimalOutputForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.iso_minimal());
}

//--------------------------------------------------------------------------
// Stamp::iso_numeric() tests
TEST(FormatTest, TestISONumericOutput)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  string n = s.iso_numeric();
  // YYYYMMDDHHMMSS (no T separator)
  EXPECT_EQ("20240615143045", n);
}

TEST(FormatTest, TestISONumericOutputForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.iso_numeric());
}

//--------------------------------------------------------------------------
// Stamp::iso_date() tests
TEST(FormatTest, TestISODateDefaultSeparator)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("2024-06-15", s.iso_date());
}

TEST(FormatTest, TestISODateCustomSeparator)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("2024/06/15", s.iso_date('/'));
}

TEST(FormatTest, TestISODateNoSeparator)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("20240615", s.iso_date(0));
}

TEST(FormatTest, TestISODateForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.iso_date());
}

//--------------------------------------------------------------------------
// Stamp::iso_time() tests
TEST(FormatTest, TestISOTimeDefaultNoSeconds)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("14:30", s.iso_time());
}

TEST(FormatTest, TestISOTimeWithSeconds)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("14:30:45", s.iso_time(':', true));
}

TEST(FormatTest, TestISOTimeNoSeparator)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("1430", s.iso_time(0));
}

TEST(FormatTest, TestISOTimeNoSeparatorWithSeconds)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_EQ("143045", s.iso_time(0, true));
}

TEST(FormatTest, TestISOTimeForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.iso_time());
}

//--------------------------------------------------------------------------
// Stamp::sql() tests
TEST(FormatTest, TestSQLOutput)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  string sql = s.sql();
  EXPECT_TRUE(sql.find("2024-06-15 14:30:45") != string::npos);
}

TEST(FormatTest, TestSQLOutputForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.sql());
}

//--------------------------------------------------------------------------
// Stamp::rfc822() tests
TEST(FormatTest, TestRFC822Output)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  string rfc = s.rfc822();
  EXPECT_TRUE(rfc.find("GMT") != string::npos);
  EXPECT_TRUE(rfc.find("Jun") != string::npos);
  EXPECT_TRUE(rfc.find("2024") != string::npos);
}

TEST(FormatTest, TestRFC822OutputForInvalidStampIsEmpty)
{
  Time::Stamp s;
  EXPECT_EQ("", s.rfc822());
}

//--------------------------------------------------------------------------
// Stamp::format() tests
TEST(FormatTest, TestFormatWithStrftimePattern)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  string f = s.format("%Y/%m/%d");
  EXPECT_EQ("2024/06/15", f);
}

//--------------------------------------------------------------------------
// Stamp::locale_date/time/date_time tests
TEST(FormatTest, TestLocaleDateReturnsNonEmpty)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_FALSE(s.locale_date().empty());
}

TEST(FormatTest, TestLocaleTimeReturnsNonEmpty)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_FALSE(s.locale_time().empty());
}

TEST(FormatTest, TestLocaleDateTimeReturnsNonEmpty)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  EXPECT_FALSE(s.locale_date_time().empty());
}

//--------------------------------------------------------------------------
// Stamp::weekday() tests
TEST(FormatTest, TestWeekdayForKnownDate)
{
  // 2024-06-15 is a Saturday = 6 (Mon=1..Sun=7)
  Time::Stamp s("2024-06-15T12:00:00Z");
  EXPECT_EQ(6, s.weekday());
}

TEST(FormatTest, TestWeekdayMonday)
{
  // 2024-06-10 is a Monday
  Time::Stamp s("2024-06-10T12:00:00Z");
  EXPECT_EQ(1, s.weekday());
}

TEST(FormatTest, TestWeekdaySunday)
{
  // 2024-06-16 is a Sunday = 7
  Time::Stamp s("2024-06-16T12:00:00Z");
  EXPECT_EQ(7, s.weekday());
}

//--------------------------------------------------------------------------
// Stamp::time() tests (convert to time_t)
TEST(FormatTest, TestTimeT)
{
  Time::Stamp s(time_t{1000000});
  EXPECT_EQ(1000000, s.time());
}

//--------------------------------------------------------------------------
// Stamp::localise() and globalise() tests
TEST(FormatTest, TestLocaliseGlobaliseRoundTrip)
{
  // In UTC, localise->globalise should round-trip (TZ=UTC in test env)
  Time::Stamp original("2024-06-15T14:30:45Z");
  Time::Stamp localised = original.localise();
  Time::Stamp globalised = localised.globalise();
  // The round-trip should give us back approximately the same time
  Time::Duration diff = globalised - original;
  EXPECT_LT(diff.seconds(), 2.0);
  EXPECT_GT(diff.seconds(), -2.0);
}

//--------------------------------------------------------------------------
// Stamp::now() tests
TEST(FormatTest, TestNowReturnsValidStamp)
{
  Time::Stamp n = Time::Stamp::now();
  EXPECT_TRUE(n.valid());
}

//--------------------------------------------------------------------------
// Stamp::operator<< tests
TEST(FormatTest, TestStampOstreamOperator)
{
  Time::Stamp s("2024-06-15T14:30:45Z");
  ostringstream oss;
  oss << s;
  EXPECT_EQ(s.iso(), oss.str());
}

//--------------------------------------------------------------------------
// Duration::hms() tests
TEST(FormatTest, TestDurationHMS)
{
  Time::Duration d(3723.0); // 1h 2m 3s
  string hms = d.hms();
  EXPECT_TRUE(hms.find("01:02:03") != string::npos);
}

TEST(FormatTest, TestDurationHMSZero)
{
  Time::Duration d(0.0);
  string hms = d.hms();
  EXPECT_TRUE(hms.find("00:00:0") != string::npos);
}

TEST(FormatTest, TestDurationHMSLargeValue)
{
  Time::Duration d(86400.0 + 3600.0); // 25 hours
  string hms = d.hms();
  EXPECT_TRUE(hms.find("25:") != string::npos);
}

//--------------------------------------------------------------------------
// Duration::iso() tests
TEST(FormatTest, TestDurationISO)
{
  Time::Duration d(90.0); // 1m 30s
  string iso = d.iso();
  EXPECT_EQ("PT1M30S", iso);
}

TEST(FormatTest, TestDurationISOZero)
{
  Time::Duration d(0.0);
  EXPECT_EQ("P0D", d.iso());
}

TEST(FormatTest, TestDurationISODaysAndHours)
{
  Time::Duration d(129600.0); // 1.5 days
  string iso = d.iso();
  EXPECT_TRUE(iso.find("P1D") != string::npos);
  EXPECT_TRUE(iso.find("12H") != string::npos);
}

TEST(FormatTest, TestDurationISOHoursOnly)
{
  Time::Duration d(7200.0);
  EXPECT_EQ("PT2H", d.iso());
}

TEST(FormatTest, TestDurationISOSecondsLargeValue)
{
  Time::Duration d(45.5);
  string iso = d.iso();
  EXPECT_TRUE(iso.find("45.5S") != string::npos);
}

//--------------------------------------------------------------------------
// Duration::clock() test
TEST(FormatTest, TestDurationClock)
{
  Time::Duration c = Time::Duration::clock();
  // clock() returns monotonic time or Duration(0) if unavailable
  // Either way it should be non-negative
  EXPECT_GE(c.seconds(), 0.0);
}

//--------------------------------------------------------------------------
// Duration operator*(double, Duration)
TEST(FormatTest, TestDurationMultiplyFromLeft)
{
  Time::Duration d(10.0);
  Time::Duration result = 3.0 * d;
  EXPECT_DOUBLE_EQ(30.0, result.seconds());
}

TEST(FormatTest, TestDurationMultiplyFromRight)
{
  Time::Duration d(10.0);
  Time::Duration result = d * 3.0;
  EXPECT_DOUBLE_EQ(30.0, result.seconds());
}

//--------------------------------------------------------------------------
// DateInterval::str() tests
TEST(FormatTest, TestDateIntervalStrDays)
{
  Time::DateInterval di(3, Time::DateInterval::days);
  EXPECT_EQ("3 days", di.str());
}

TEST(FormatTest, TestDateIntervalStrSingularDay)
{
  Time::DateInterval di(1, Time::DateInterval::days);
  EXPECT_EQ("1 day", di.str());
}

TEST(FormatTest, TestDateIntervalStrWeeks)
{
  Time::DateInterval di(2, Time::DateInterval::weeks);
  EXPECT_EQ("2 weeks", di.str());
}

TEST(FormatTest, TestDateIntervalStrSingularWeek)
{
  Time::DateInterval di(1, Time::DateInterval::weeks);
  EXPECT_EQ("1 week", di.str());
}

TEST(FormatTest, TestDateIntervalStrMonths)
{
  Time::DateInterval di(6, Time::DateInterval::months);
  EXPECT_EQ("6 months", di.str());
}

TEST(FormatTest, TestDateIntervalStrSingularMonth)
{
  Time::DateInterval di(1, Time::DateInterval::months);
  EXPECT_EQ("1 month", di.str());
}

TEST(FormatTest, TestDateIntervalStrYears)
{
  Time::DateInterval di(5, Time::DateInterval::years);
  EXPECT_EQ("5 years", di.str());
}

TEST(FormatTest, TestDateIntervalStrInvalid)
{
  Time::DateInterval di;
  EXPECT_EQ("INVALID", di.str());
}

//--------------------------------------------------------------------------
// DateInterval::operator<< tests
TEST(FormatTest, TestDateIntervalOstreamOperator)
{
  Time::DateInterval di(3, Time::DateInterval::days);
  ostringstream oss;
  oss << di;
  EXPECT_EQ("3 days", oss.str());
}

//--------------------------------------------------------------------------
// DateStamp::iso() and operator<< tests
TEST(FormatTest, TestDateStampISO)
{
  Time::DateStamp ds("2024-06-15");
  EXPECT_EQ("2024-06-15", ds.iso());
}

TEST(FormatTest, TestDateStampOstreamOperator)
{
  Time::DateStamp ds("2024-06-15");
  ostringstream oss;
  oss << ds;
  EXPECT_EQ("2024-06-15", oss.str());
}

//--------------------------------------------------------------------------
// Split::operator<< tests
TEST(FormatTest, TestSplitOstreamOperator)
{
  Time::Split sp(2024, 6, 15, 14, 30, 45);
  ostringstream oss;
  oss << sp;
  string out = oss.str();
  EXPECT_TRUE(out.find("2024") != string::npos);
  EXPECT_TRUE(out.find("14") != string::npos);
}

//--------------------------------------------------------------------------
// DateInterval add_to/subtract_from with weeks
TEST(FormatTest, TestDateIntervalAddWeeks)
{
  Time::Split split1(2024, 1, 1, 0, 0, 0);
  Time::DateInterval di(2, Time::DateInterval::weeks);
  di.add_to(split1);
  EXPECT_EQ(15, split1.day);
}

TEST(FormatTest, TestDateIntervalSubtractWeeks)
{
  Time::Split split1(2024, 1, 15, 0, 0, 0);
  Time::DateInterval di(2, Time::DateInterval::weeks);
  di.subtract_from(split1);
  EXPECT_EQ(1, split1.day);
}

//--------------------------------------------------------------------------
// Parse edge cases to cover remaining error paths

// ISO date-only, non-lenient → no T/space after date → returns false
TEST(FormatTest, TestParseDateOnlyNonLenientFails)
{
  Time::Stamp s("2024-06-15");
  EXPECT_FALSE(s.valid());
}

// ISO with HH:MM but no seconds, lenient → triggers read_part_f pos==size
TEST(FormatTest, TestParseLenientHHMMNoSeconds)
{
  Time::Stamp s("2024-06-15T14:30", true);
  EXPECT_TRUE(s.valid());
  // Should parse as 14:30:00
  string iso = s.iso();
  EXPECT_TRUE(iso.find("14:30:00") != string::npos);
}

// ISO with non-digit char in seconds field → triggers read_part_f bad char
TEST(FormatTest, TestParseISOBadSecondsChar)
{
  Time::Stamp s("2024-06-15T14:30:5X");
  EXPECT_FALSE(s.valid());
}

// RFC822 with malformed time → triggers read_time failure in read_rfc_822
TEST(FormatTest, TestParseRFC822BadTime)
{
  Time::Stamp s("Sun, 06 Nov 1994 XX:XX:XX GMT");
  EXPECT_FALSE(s.valid());
}

// RFC850 with malformed time → triggers read_time failure in read_rfc_850
TEST(FormatTest, TestParseRFC850BadTime)
{
  Time::Stamp s("Sunday, 06-Nov-94 XX:XX:XX GMT");
  EXPECT_FALSE(s.valid());
}

// asctime with malformed time → triggers read_time failure in read_asctime
TEST(FormatTest, TestParseAsctimeBadTime)
{
  Time::Stamp s("Sun Nov 6 XX:XX:XX 1994");
  EXPECT_FALSE(s.valid());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
