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

TEST(DateIntervalTest, Test_adding_to_split_days)
{
  Time::Split split1(1967, 1, 29, 6, 42, 01);
  Time::Split split2(1967, 2, 01, 6, 42, 01);
  DateInterval di("3 days");
  di.add_to(split1);
  EXPECT_EQ(split2, split1);

  Time::DateStamp s1("1967-01-29");
  Time::DateStamp s2("1967-02-01");
  EXPECT_EQ(s2, s1+di);
  s1 += di;
  EXPECT_EQ(s2, s1);
}

TEST(DateIntervalTest, Test_adding_to_split_months)
{
  Time::Split split1(1967, 1, 29, 6, 42, 01);
  Time::Split split2(2017, 1, 29, 6, 42, 01);
  DateInterval di("600 months");
  di.add_to(split1);
  EXPECT_EQ(split2, split1);

  Time::DateStamp s1("1967-01-29");
  Time::DateStamp s2("2017-01-29");
  EXPECT_EQ(s2, s1+di);
  s1 += di;
  EXPECT_EQ(s2, s1);
}

TEST(DateIntervalTest, Test_adding_to_split_years)
{
  Time::Split split1(1967, 1, 29, 6, 42, 01);
  Time::Split split2(2017, 1, 29, 6, 42, 01);
  DateInterval di("50 years");
  di.add_to(split1);
  EXPECT_EQ(split2, split1);

  Time::DateStamp s1("1967-01-29");
  Time::DateStamp s2("2017-01-29");
  EXPECT_EQ(s2, s1+di);
  s1 += di;
  EXPECT_EQ(s2, s1);
}

TEST(DateIntervalTest, Test_subtracting_from_split_days)
{
  Time::Split split1(1967, 2, 01, 6, 42, 01);
  Time::Split split2(1967, 1, 29, 6, 42, 01);
  DateInterval di("3 days");
  di.subtract_from(split1);
  EXPECT_EQ(split2, split1);

  Time::DateStamp s1("1967-02-01");
  Time::DateStamp s2("1967-01-29");
  EXPECT_EQ(s2, s1-di);
  s1 -= di;
  EXPECT_EQ(s2, s1);
}

TEST(DateIntervalTest, Test_subtracting_from_split_months)
{
  Time::Split split1(2017, 1, 29, 6, 42, 01);
  Time::Split split2(1967, 1, 29, 6, 42, 01);
  DateInterval di("600 months");
  di.subtract_from(split1);
  EXPECT_EQ(split2, split1);

  Time::DateStamp s1("2017-01-29");
  Time::DateStamp s2("1967-01-29");
  EXPECT_EQ(s2, s1-di);
  s1 -= di;
  EXPECT_EQ(s2, s1);
}

TEST(DateIntervalTest, Test_subtracting_from_split_years)
{
  Time::Split split1(2017, 1, 29, 6, 42, 01);
  Time::Split split2(1967, 1, 29, 6, 42, 01);
  DateInterval di("50 years");
  di.subtract_from(split1);
  EXPECT_EQ(split2, split1);

  Time::DateStamp s1("2017-01-29");
  Time::DateStamp s2("1967-01-29");
  EXPECT_EQ(s2, s1-di);
  s1 -= di;
  EXPECT_EQ(s2, s1);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



