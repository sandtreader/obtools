//==========================================================================
// ObTools::Time: test-split.cc
//
// Test harness for split() functions
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"
#include <iostream>

using namespace std;
using namespace ObTools;

// Generic split test - check string format splits to correct values
void test_split(const string& str, int year, int month, int day,
                int hour = 0, int min = 0, int sec = 0)
{
  // Auto add time if not given
  string s = str;
  if (s.size() < 11) s+="T00:00:00";

  Time::Stamp t(s);
  Time::Split split;
  t.split(split);

  EXPECT_EQ(split.year, year);
  EXPECT_EQ(split.month, month);
  EXPECT_EQ(split.day, day);
  EXPECT_EQ(split.hour, hour);
  EXPECT_EQ(split.min, min);
  EXPECT_EQ(split.sec, sec);
}

TEST(SplitTests, TestZeroDay)
{
  test_split("1900-01-01", 1900, 1, 1);
}

TEST(SplitTests, TestPaulsBirthday)
{
  test_split("1967-01-29", 1967, 1, 29);
}

TEST(SplitTests, TestSplitAroundEndOfYearBeforeLeapYear)
{
  // This is the earliest time on 2023-12-31 which will trigger the bug
  test_split("2023-12-31T18:15:00Z", 2023, 12, 31, 18, 15, 0);
  test_split("2023-12-31T23:59:59Z", 2023, 12, 31, 23, 59, 59);
}

TEST(SplitTests, TestDayBeforeLeapDay)
{
  test_split("2012-02-28", 2012, 2, 28);
}

TEST(SplitTests, TestFirstDaysOfLeapYear)
{
  test_split("2024-01-01", 2024, 1, 1);
  test_split("2024-01-02", 2024, 1, 2);
}

TEST(SplitTests, TestLastDaysOfLeapYear)
{
  test_split("2024-12-31", 2024, 12, 31);
  test_split("2025-01-01", 2025, 1, 1);
}

TEST(SplitTests, Test1900NotLeapYear)
{
  test_split("1900-03-01", 1900, 3, 1);
}

TEST(SplitTests, Test1904WasLeapYear)
{
  test_split("1904-02-29", 1904, 2, 29);
}

TEST(SplitTests, Test2000WasLeapYear)
{
  test_split("2000-02-29", 2000, 2, 29);
}

TEST(SplitTests, TestLeapDay)
{
  test_split("2012-02-29", 2012, 2, 29);
}

TEST(SplitTests, TestAfterNTPRollover)
{
  test_split("2036-02-08", 2036, 2, 8);
}

TEST(SplitTests, TestAfterTimeTRollover)
{
  test_split("2038-01-01", 2038, 1, 1);
}

TEST(SplitTests, TestAVeryLongTimeAway)
{
  test_split("4000-01-01", 4000, 1, 1);
}

TEST(SplitTests, TestTimes)
{
  test_split("2011-09-29T17:14:23", 2011, 9, 29, 17, 14, 23);
  test_split("2011-09-29T23:59:59", 2011, 9, 29, 23, 59, 59);
  test_split("2011-09-29T24:00:00", 2011, 9, 30, 00, 00, 00);
}

TEST(SplitTests, Test_normalisation_when_not_required)
{
  Time::Split split1(1967, 1, 29, 6, 42, 1);  // Already normalised
  Time::Split split2 = split1;
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_seconds)
{
  Time::Split split1(1967, 1, 29, 6, 42, -61);
  Time::Split split2(1967, 1, 29, 6, 40, 59);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_seconds)
{
  Time::Split split1(1967, 1, 29, 6, 42, 61);
  Time::Split split2(1967, 1, 29, 6, 43, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_minutes)
{
  Time::Split split1(1967, 1, 29, 6, -61, 01);
  Time::Split split2(1967, 1, 29, 4, 59, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_minutes)
{
  Time::Split split1(1967, 1, 29, 6, 102, 01);
  Time::Split split2(1967, 1, 29, 7, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_hours)
{
  Time::Split split1(1967, 1, 29, -6, 42, 01);
  Time::Split split2(1967, 1, 28, 18, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_hours)
{
  Time::Split split1(1967, 1, 29, 30, 42, 01);
  Time::Split split2(1967, 1, 30, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_days_31_month)
{
  Time::Split split1(1967, 2, -2, 6, 42, 01);
  Time::Split split2(1967, 1, 29, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_days_31_month)
{
  Time::Split split1(1967, 1, 32, 6, 42, 01);
  Time::Split split2(1967, 2, 01, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_days_28_month)
{
  Time::Split split1(1967, 3, -2, 6, 42, 01);
  Time::Split split2(1967, 2, 26, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_days_28_month)
{
  Time::Split split1(1967, 2, 32, 6, 42, 01);
  Time::Split split2(1967, 3, 4, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_days_leap_year)
{
  Time::Split split1(2000, 3, -2, 6, 42, 01);
  Time::Split split2(2000, 2, 27, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_days_leap_year)
{
  Time::Split split1(2000, 2, 32, 6, 42, 01);
  Time::Split split2(2000, 3, 3, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_months)
{
  Time::Split split1(1967, -9, 29, 6, 42, 01);
  Time::Split split2(1966, 3, 29, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_months)
{
  Time::Split split1(1967, 601, 29, 6, 42, 01);
  Time::Split split2(2017, 1, 29, 6, 42, 01);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_negative_ripple)
{
  Time::Split split1(2000, 1, 1, 0, 0, -1);
  Time::Split split2(1999, 12, 31, 23, 59, 59);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

TEST(SplitTests, Test_normalisation_overflow_ripple)
{
  Time::Split split1(1999, 12, 31, 23, 59, 60);
  Time::Split split2(2000, 1, 1, 0, 0, 0);
  split1.normalise();
  EXPECT_EQ(split2, split1);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



