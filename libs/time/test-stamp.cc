//==========================================================================
// ObTools::Time: test-stamp.cc
//
// Test Stamp functions
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"

using namespace std;
using namespace ObTools;

TEST(StampTest, TestCreateFromJulianDays)
{
  EXPECT_EQ(Time::Stamp{}, Time::Stamp::from_jdn(2415021.5));
  EXPECT_EQ(Time::Stamp{time_t{0}}, Time::Stamp::from_jdn(2440588.5));
  EXPECT_EQ(Time::Stamp{"1972-01-01T00:00:00Z"},
            Time::Stamp::from_jdn(2441318.5));
  EXPECT_EQ(Time::Stamp{"2036-02-07T00:00:00Z"},
            Time::Stamp::from_jdn(2464731.5));
}

TEST(StampTest, TestCreateToJulianDays)
{
  EXPECT_EQ(2415021.5, Time::Stamp{}.jdn());
  EXPECT_EQ(2440588.5, Time::Stamp{time_t{0}}.jdn());
  EXPECT_EQ(2441318.5, Time::Stamp{"1972-01-01T00:00:00Z"}.jdn());
  EXPECT_EQ(2464731.5, Time::Stamp{"2036-02-07T00:00:00Z"}.jdn());
}

TEST(StampTest, TestStampSubtraction)
{
  Time::Stamp s1{"1967-01-29T06:00:00Z"};
  Time::Stamp s2{"1967-01-29T07:30:00Z"};
  Time::Duration d = s2-s1;
  EXPECT_EQ(5400.0, d.seconds());
}

TEST(StampTest, TestStampAdditionOfDuration)
{
  Time::Stamp s1{"1967-01-29T06:00:00Z"};
  Time::Stamp s2{"1967-01-29T07:30:00Z"};
  Time::Duration d(5400.0);
  EXPECT_EQ(s2, s1+d);
  s1+=d;
  EXPECT_EQ(s2, s1);
}

TEST(StampTest, TestStampSubtractionOfDuration)
{
  Time::Stamp s1{"1967-01-29T06:00:00Z"};
  Time::Stamp s2{"1967-01-29T07:30:00Z"};
  Time::Duration d(5400.0);
  EXPECT_EQ(s1, s2-d);
  s2-=d;
  EXPECT_EQ(s1, s2);
}

TEST(StampTest, TestStampAdditionOfNegativeDuration)
{
  Time::Stamp s1{"1967-01-29T06:00:00Z"};
  Time::Stamp s2{"1967-01-29T07:30:00Z"};
  Time::Duration d(-5400.0);
  EXPECT_EQ(s1, s2+d);
  s2+=d;
  EXPECT_EQ(s1, s2);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



