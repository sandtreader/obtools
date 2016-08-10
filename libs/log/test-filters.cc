//==========================================================================
// ObTools::Log: test-filters.cc
//
// Test harness for log filters
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-log.h"

using namespace std;
using namespace ObTools;

TEST(LogFilters, TestSecondsRoundedDown)
{
  ostringstream oss;
  Log::StreamChannel sc(oss);
  Log::TimestampFilter filter("%*S: ", sc);

  Log::Message msg(Log::LEVEL_DETAIL, "Hello!");
  msg.timestamp = Time::Stamp("1967-01-29 05:59:59.9997");

  filter.log(msg);
  ASSERT_EQ("59.999: Hello!\n", oss.str());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
