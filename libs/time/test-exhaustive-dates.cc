//==========================================================================
// ObTools::Time: test-exhaustive-dates.cc
//
// Test all dates function
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"
#include <iostream>
#include <limits.h>

using namespace std;
using namespace ObTools;

TEST(SplitTests, TestExhaustiveDates)
{
  Time::Stamp original;
  Time::Duration a_day("1 day");

  // Number of days from 1900 to 2036 end of NTP time we can test,
  // and then some to test rollover then, and in 2037
  int ndays = UINT_MAX / Time::DAY + 1000;

  for(int i=0; i<ndays; i++)
  {
    Time::Split split;
    original.split(split);

    EXPECT_EQ(split.hour, 0) << original.iso();
    EXPECT_EQ(split.min,  0) << original.iso();
    EXPECT_EQ(split.sec,  0) << original.iso();

    Time::Stamp recombined(split);

    EXPECT_EQ(original.ntp(), recombined.ntp());
    EXPECT_EQ(original.iso(), recombined.iso());

    original += a_day;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



