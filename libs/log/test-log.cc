//==========================================================================
// ObTools::Log: test-log.cc
//
// Test harness for logging
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-log.h"

using namespace std;
using namespace ObTools;

TEST(LogFilters, TestMessageLogged)
{
  ostringstream oss;
  auto *sc = new Log::StreamChannel{&oss};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  Log::Distributor dtr{};
  dtr.connect(sc);
  dtr.log(msg);
  ASSERT_EQ("Hello!\n", oss.str());
}

TEST(LogFilters, TestMessageLoggedWithFullConnect)
{
  ostringstream oss;
  auto *sc = new Log::StreamChannel{&oss};
  auto msg = Log::Message{Log::Level::detail, "Hello!"};
  msg.timestamp = Time::Stamp{"1967-01-29 06:00:00"};
  Log::Distributor dtr{};
  dtr.connect_full(sc, Log::Level::detail, "%H:%M:%S [%*L]: ");
  dtr.log(msg);
  ASSERT_EQ("06:00:00 [3]: Hello!\n", oss.str());
}

TEST(LogFilters, TestMessagesLoggedOkFromConcurrentThreads)
{
  stringstream ss;
  auto *sc = new Log::StreamChannel{&ss};
  Log::Distributor dtr{};
  dtr.connect(sc);

  auto threads = list<thread>{};
  for (auto i = 0; i < 100; ++i)
    threads.push_back(thread{[&dtr]()
        {
          for (auto i = 0; i < 100; ++i)
          {
            auto msg = Log::Message{Log::Level::detail,
                                    "This is not interleaved"};
            dtr.log(msg);
          }
        }
      });

  for (auto& t: threads)
    t.join();

  string line;
  for (auto i = 0; i < 10000; ++i)
  {
    getline(ss, line);
    ASSERT_EQ("This is not interleaved", line) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
