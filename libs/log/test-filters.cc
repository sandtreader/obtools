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

class Collector: public Log::Channel
{
public:
  vector<string> msgs;
  void log(const Log::Message& msg) { msgs.push_back(msg.text); }
};

TEST(LogFilters, TestLevelFilterDoesNotAlterMessage)
{
  auto collector = Collector{};
  auto filter = Log::LevelFilter{&collector, Log::Level::detail};
  const auto expected = "Hello!";
  auto msg = Log::Message{Log::Level::detail, expected};
  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ(expected, collector.msgs[0]);
}

TEST(LogFilters, TestLevelFilterAllowsHighEnoughMessage)
{
  auto collector = Collector{};
  auto filter = Log::LevelFilter{&collector, Log::Level::detail};
  const auto expected = "Hello!";
  auto msg = Log::Message{Log::Level::summary, "Hello!"};
  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ(expected, collector.msgs[0]);
}

TEST(LogFilters, TestLevelFilterDropsTooLowMessage)
{
  auto collector = Collector{};
  auto filter = Log::LevelFilter{&collector, Log::Level::summary};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  filter.log(msg);
  ASSERT_TRUE(collector.msgs.empty());
}

TEST(LogFilters, TestPatternFilterDoesNotAlterMessage)
{
  auto collector = Collector{};
  auto filter = Log::PatternFilter{&collector, "H*"};
  const auto expected = "Hello!";
  auto msg = Log::Message{Log::Level::detail, expected};
  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ(expected, collector.msgs[0]);
}

TEST(LogFilters, TestPatternFilterAllowsMatchingMessage)
{
  auto collector = Collector{};
  auto filter = Log::PatternFilter{&collector, "H*"};
  const auto expected = "Hello!";
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ(expected, collector.msgs[0]);
}

TEST(LogFilters, TestPatternFilterDropsNonMatchingMessage)
{
  auto collector = Collector{};
  auto filter = Log::PatternFilter{&collector, "F*"};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  filter.log(msg);
  ASSERT_TRUE(collector.msgs.empty());
}

TEST(LogFilters, TestTimeStampFilterAddsTimestamp)
{
  auto collector = Collector{};
  auto filter = Log::TimestampFilter{&collector, "%H:%M:%*S %a %d %b %Y: "};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  msg.timestamp = Time::Stamp{"1967-01-29 06:01:45"};

  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ("06:01:45.000 Sun 29 Jan 1967: Hello!", collector.msgs[0]);
}

TEST(LogFilters, TestTimeStampFilterAddsLogLevelMaybeItShouldntThough)
{
  auto collector = Collector{};
  auto filter = Log::TimestampFilter{&collector, "[%*L]: "};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};

  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ("[3]: Hello!", collector.msgs[0]);
}

TEST(LogFilters, TestTimeStampFilterSecondsRoundedDown)
{
  auto collector = Collector{};
  auto filter = Log::TimestampFilter{&collector, "%*S: "};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  msg.timestamp = Time::Stamp{"1967-01-29 05:59:59.9997"};

  filter.log(msg);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ("59.999: Hello!", collector.msgs[0]);
}

TEST(LogFilters, TestRepeatedMessageFilterPassesDifferentMessages)
{
  auto collector = Collector{};
  auto filter = Log::RepeatedMessageFilter{&collector};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  auto msg2 = Log::Message{Log::Level::detail, "Goodbye!"};

  filter.log(msg1);
  filter.log(msg2);
  ASSERT_EQ(2, collector.msgs.size());
  ASSERT_EQ("Hello!", collector.msgs[0]);
  ASSERT_EQ("Goodbye!", collector.msgs[1]);
}

TEST(LogFilters, TestRepeatedMessageFilterSuppressesSameMessage)
{
  auto collector = Collector{};
  auto filter = Log::RepeatedMessageFilter{&collector};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};

  filter.log(msg1);
  filter.log(msg1);
  ASSERT_EQ(1, collector.msgs.size());
  ASSERT_EQ("Hello!", collector.msgs[0]);
}

TEST(LogFilters, TestRepeatedMessageFilterPreservesMessageOnOnlyOneRepeat)
{
  auto collector = Collector{};
  auto filter = Log::RepeatedMessageFilter{&collector};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  auto msg2 = Log::Message{Log::Level::detail, "Goodbye!"};

  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg2);
  ASSERT_EQ(3, collector.msgs.size());
  ASSERT_EQ("Hello!", collector.msgs[0]);
  ASSERT_EQ("Hello!", collector.msgs[1]);
  ASSERT_EQ("Goodbye!", collector.msgs[2]);
}

TEST(LogFilters, TestRepeatedMessageFilterShowsReportOnMoreThanOneRepeat)
{
  auto collector = Collector{};
  auto filter = Log::RepeatedMessageFilter{&collector};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  auto msg2 = Log::Message{Log::Level::detail, "Goodbye!"};

  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg2);
  ASSERT_EQ(3, collector.msgs.size());
  ASSERT_EQ("Hello!", collector.msgs[0]);
  ASSERT_EQ("(2 identical messages suppressed)", collector.msgs[1]);
  ASSERT_EQ("Goodbye!", collector.msgs[2]);
}

TEST(LogFilters, TestRepeatedMessageFilterReportsAfterHoldTime)
{
  auto collector = Collector{};
  auto filter = Log::RepeatedMessageFilter{&collector};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  msg1.timestamp = Time::Stamp{"1967-01-29 06:00:00"};
  auto msg2 = Log::Message{Log::Level::detail, "Hello!"};
  msg2.timestamp = Time::Stamp{"1967-01-29 06:00:10"};

  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg2);
  ASSERT_EQ(2, collector.msgs.size());
  ASSERT_EQ("Hello!", collector.msgs[0]);
  ASSERT_EQ("(3 identical messages suppressed)", collector.msgs[1]);
}

TEST(LogFilters, TestRepeatedMessageFilterReportsAfterHoldTimeAndContinues)
{
  auto collector = Collector{};
  auto filter = Log::RepeatedMessageFilter{&collector};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  msg1.timestamp = Time::Stamp{"1967-01-29 06:00:00"};
  auto msg2 = Log::Message{Log::Level::detail, "Hello!"};
  msg2.timestamp = Time::Stamp{"1967-01-29 06:00:10"};
  auto msg3 = Log::Message{Log::Level::detail, "Hello!"};
  msg3.timestamp = Time::Stamp{"1967-01-29 06:00:20"};

  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg2);
  filter.log(msg2);
  filter.log(msg2);
  filter.log(msg3);
  ASSERT_EQ(3, collector.msgs.size());
  ASSERT_EQ("Hello!", collector.msgs[0]);
  ASSERT_EQ("(3 identical messages suppressed)", collector.msgs[1]);
  ASSERT_EQ("(3 identical messages suppressed)", collector.msgs[2]);
}

TEST(LogFilters,
     TestRepeatedMessageFilterSetsTimestampToLastSuppressedWhenDifferent)
{
  auto collector = Collector{};
  auto tsfilter = Log::TimestampFilter{&collector, "%H:%M:%S "};
  auto filter = Log::RepeatedMessageFilter{&tsfilter};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  msg1.timestamp = Time::Stamp{"1967-01-29 06:00:00"};
  auto msg2 = Log::Message{Log::Level::detail, "Goodbye!"};
  msg2.timestamp = Time::Stamp{"1967-01-29 06:00:10"};

  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg2);
  ASSERT_EQ(3, collector.msgs.size());
  ASSERT_EQ("06:00:00 Hello!", collector.msgs[0]);
  ASSERT_EQ("06:00:00 (2 identical messages suppressed)", collector.msgs[1]);
  ASSERT_EQ("06:00:10 Goodbye!", collector.msgs[2]);
}

TEST(LogFilters, TestRepeatedMessageFilterSetsTimestampToCurrentWhenSame)
{
  auto collector = Collector{};
  auto tsfilter = Log::TimestampFilter{&collector, "%H:%M:%S "};
  auto filter = Log::RepeatedMessageFilter{&tsfilter};
  auto msg1 = Log::Message{Log::Level::detail, "Hello!"};
  msg1.timestamp = Time::Stamp{"1967-01-29 06:00:00"};
  auto msg2 = Log::Message{Log::Level::detail, "Hello!"};
  msg2.timestamp = Time::Stamp{"1967-01-29 06:00:10"};

  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg1);
  filter.log(msg2);
  ASSERT_EQ(2, collector.msgs.size());
  ASSERT_EQ("06:00:00 Hello!", collector.msgs[0]);
  ASSERT_EQ("06:00:10 (3 identical messages suppressed)", collector.msgs[1]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
