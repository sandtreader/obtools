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
  EXPECT_EQ(Time::Stamp{}, Time::Stamp(2415021.5));
  EXPECT_EQ(Time::Stamp{time_t{0}}, Time::Stamp(2440588.5));
  EXPECT_EQ(Time::Stamp{"1972-01-01T00:00:00Z"}, Time::Stamp(2441318.5));
  EXPECT_EQ(Time::Stamp{"2036-02-07T00:00:00Z"}, Time::Stamp(2464731.5));
}

TEST(StampTest, TestCreateToJulianDays)
{
  EXPECT_EQ(2415021.5, Time::Stamp{}.jdn());
  EXPECT_EQ(2440588.5, Time::Stamp{time_t{0}}.jdn());
  EXPECT_EQ(2441318.5, Time::Stamp{"1972-01-01T00:00:00Z"}.jdn());
  EXPECT_EQ(2464731.5, Time::Stamp{"2036-02-07T00:00:00Z"}.jdn());
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



