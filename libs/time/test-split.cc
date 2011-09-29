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
  test_split("2011-12-05", 2011, 12, 5);
}

TEST(SplitTests, TestDayBeforeLeapDay)
{
  test_split("2012-02-28", 2012, 2, 28);
}

TEST(SplitTests, TestFirstDayOfLeapYear)
{
  test_split("2012-01-01", 2012, 1, 1);
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

TEST(SplitTests, TestTimes)
{
  test_split("2011-09-29T17:14:23", 2011, 9, 29, 17, 14, 23);
  test_split("2011-09-29T23:59:59", 2011, 9, 29, 23, 59, 59);
  test_split("2011-09-29T24:00:00", 2011, 9, 30, 00, 00, 00);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



