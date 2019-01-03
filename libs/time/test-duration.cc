//==========================================================================
// ObTools::Time: test-duration.cc
//
// Test Duration functions
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"

using namespace std;
using namespace ObTools;

TEST(DurationTest, TestConvertToNTP)
{
  Time::Duration d(3600);
  EXPECT_EQ(15461882265600ULL, d.ntp());
}

TEST(DurationTest, TestConvertToNTPNegative)
{
  Time::Duration d(-3600);
  EXPECT_EQ(18446728611827286016ULL, d.ntp());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



