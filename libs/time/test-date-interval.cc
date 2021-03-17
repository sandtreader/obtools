//==========================================================================
// ObTools::Time: test-date-interval.cc
//
// Test DateInterval functions
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Time;

TEST(DateIntervalTest, Test_default_construction)
{
  DateInterval di;
  ASSERT_TRUE(!di);
  EXPECT_EQ(0, di.number);
  EXPECT_EQ(DateInterval::invalid, di.unit);
}

TEST(DateIntervalTest, Test_basic_construction)
{
  DateInterval di(3, DateInterval::days);
  ASSERT_FALSE(!di);
  EXPECT_EQ(3, di.number);
  EXPECT_EQ(DateInterval::days, di.unit);
}

TEST(DateIntervalTest, Test_number_and_string_construction)
{
  DateInterval di(3, "days");
  ASSERT_FALSE(!di);
  EXPECT_EQ(3, di.number);
  EXPECT_EQ(DateInterval::days, di.unit);
}

TEST(DateIntervalTest, Test_string_construction)
{
  DateInterval di("3 days");
  ASSERT_FALSE(!di);
  EXPECT_EQ(3, di.number);
  EXPECT_EQ(DateInterval::days, di.unit);
}

TEST(DateIntervalTest, Test_bad_string_construction)
{
  DateInterval di1("42");
  ASSERT_TRUE(!di1);

  DateInterval di2("");
  ASSERT_TRUE(!di2);
}

TEST(DateIntervalTest, Test_all_unit_strings)
{
  EXPECT_EQ(DateInterval::days, DateInterval::get_unit("days"));
  EXPECT_EQ(DateInterval::days, DateInterval::get_unit("day"));
  EXPECT_EQ(DateInterval::days, DateInterval::get_unit("d"));
  EXPECT_EQ(DateInterval::days, DateInterval::get_unit("DAYS"));
  EXPECT_EQ(DateInterval::days, DateInterval::get_unit("DAY"));
  EXPECT_EQ(DateInterval::days, DateInterval::get_unit("D"));

  EXPECT_EQ(DateInterval::weeks, DateInterval::get_unit("weeks"));
  EXPECT_EQ(DateInterval::weeks, DateInterval::get_unit("week"));
  EXPECT_EQ(DateInterval::weeks, DateInterval::get_unit("w"));
  EXPECT_EQ(DateInterval::weeks, DateInterval::get_unit("WEEKS"));
  EXPECT_EQ(DateInterval::weeks, DateInterval::get_unit("Week"));
  EXPECT_EQ(DateInterval::weeks, DateInterval::get_unit("W"));

  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("months"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("month"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("mon"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("m"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("MONTHS"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("Month"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("MON"));
  EXPECT_EQ(DateInterval::months, DateInterval::get_unit("M"));

  EXPECT_EQ(DateInterval::years, DateInterval::get_unit("years"));
  EXPECT_EQ(DateInterval::years, DateInterval::get_unit("year"));
  EXPECT_EQ(DateInterval::years, DateInterval::get_unit("y"));
  EXPECT_EQ(DateInterval::years, DateInterval::get_unit("YEARS"));
  EXPECT_EQ(DateInterval::years, DateInterval::get_unit("Year"));
  EXPECT_EQ(DateInterval::years, DateInterval::get_unit("Y"));

  EXPECT_EQ(DateInterval::invalid, DateInterval::get_unit(""));
  EXPECT_EQ(DateInterval::invalid, DateInterval::get_unit("foo"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



