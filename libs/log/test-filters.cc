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

TEST(LogFilters, TestLevelFilterDoesNotAlterMessage)
{
  auto filter = Log::LevelFilter{Log::Level::detail};
  const auto expected = "Hello!";
  auto msg = Log::Message{Log::Level::detail, expected};
  filter.pass(msg);
  ASSERT_EQ(expected, msg.text);
}

TEST(LogFilters, TestLevelFilterAllowsHighEnoughMessage)
{
  auto filter = Log::LevelFilter{Log::Level::detail};
  auto msg = Log::Message{Log::Level::summary, "Hello!"};
  ASSERT_TRUE(filter.pass(msg));
}

TEST(LogFilters, TestLevelFilterDropsTooLowMessage)
{
  auto filter = Log::LevelFilter{Log::Level::summary};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  ASSERT_FALSE(filter.pass(msg));
}

TEST(LogFilters, TestPatternFilterDoesNotAlterMessage)
{
  auto filter = Log::PatternFilter{"H*"};
  const auto expected = "Hello!";
  auto msg = Log::Message{Log::Level::detail, expected};
  filter.pass(msg);
  ASSERT_EQ(expected, msg.text);
}

TEST(LogFilters, TestPatternFilterAllowsMatchingMessage)
{
  auto filter = Log::PatternFilter{"H*"};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  ASSERT_TRUE(filter.pass(msg));
}

TEST(LogFilters, TestPatternFilterDropsNonMatchingMessage)
{
  auto filter = Log::PatternFilter{"F*"};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  ASSERT_FALSE(filter.pass(msg));
}

TEST(LogFilters, TestTimeStampFilterAddsTimestamp)
{
  auto filter = Log::TimestampFilter{"%H:%M:%*S %a %d %b %Y: "};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  msg.timestamp = Time::Stamp{"1967-01-29 06:01:45"};

  filter.pass(msg);
  ASSERT_EQ("06:01:45.000 Sun 29 Jan 1967: Hello!", msg.text);
}

TEST(LogFilters, TestTimeStampFilterAddsLogLevelMaybeItShouldntThough)
{
  auto filter = Log::TimestampFilter{"[%*L]: "};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};

  filter.pass(msg);
  ASSERT_EQ("[3]: Hello!", msg.text);
}

TEST(LogFilters, TestTimeStampFilterSecondsRoundedDown)
{
  auto filter = Log::TimestampFilter{"%*S: "};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  msg.timestamp = Time::Stamp{"1967-01-29 05:59:59.9997"};

  filter.pass(msg);
  ASSERT_EQ("59.999: Hello!", msg.text);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
