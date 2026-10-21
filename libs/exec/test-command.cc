//==========================================================================
// ObTools::Exec: test-command.cc
//
// Test harness for command execution
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-exec.h"
#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace ObTools;
using namespace ObTools::Exec;

TEST(Command, TestFailureOnBadCommand)
{
  Command cmd("this-will-fail");
  ASSERT_FALSE(cmd.execute());
}

TEST(Command, TestOKOnSimpleGoodCommand)
{
  Command cmd("/bin/date");
  ASSERT_TRUE(cmd.execute());
}

TEST(Command, TestOutputFromCommandWithArguments)
{
  Command cmd("/bin/echo Hello, world!");
  string output;
  ASSERT_TRUE(cmd.execute(output));
  ASSERT_EQ("Hello, world!\n", output);
}

TEST(Command, TestInputToCommand)
{
  Command cmd("/bin/cat");
  string output;
  ASSERT_TRUE(cmd.execute("Hello, world!", output));
  ASSERT_EQ("Hello, world!", output);
}

TEST(Command, TestNoPipesLeakIntoConcurrentChildren)
{
  // Another command in progress in a thread, with its pipes open
  thread other([]()
  {
    Command cmd("/bin/sleep 1");
    cmd.execute();
  });
  this_thread::sleep_for(chrono::milliseconds{300});

  // This child must only have stdin, stdout and stderr (plus ls's own fd
  // for reading /proc/self/fd), not the other command's pipes
  Command cmd("/bin/ls /proc/self/fd");
  string output;
  EXPECT_TRUE(cmd.execute(output));
  other.join();
  EXPECT_EQ("0\n1\n2\n3\n", output);
}

TEST(Command, TestConcurrentInputAndOutput)
{
  // Many commands at once, each passing its own input through cat - each
  // must get EOF on its input promptly and return exactly its own output
  const auto n = 16;
  vector<string> inputs(n), outputs(n);
  vector<int> results(n);
  vector<thread> threads;
  for(auto i=0; i<n; i++)
    inputs[i] = string(100000 + i, 'a' + i);

  const auto start = chrono::steady_clock::now();
  for(auto i=0; i<n; i++)
    threads.emplace_back([&, i]()
    {
      Command cmd("/usr/bin/timeout 5 /bin/cat");
      results[i] = cmd.execute(inputs[i], outputs[i]);
    });
  for(auto& t: threads) t.join();
  const auto elapsed = chrono::steady_clock::now() - start;

  for(auto i=0; i<n; i++)
  {
    EXPECT_TRUE(results[i]) << i;
    EXPECT_EQ(inputs[i].size(), outputs[i].size()) << i;
    EXPECT_TRUE(inputs[i] == outputs[i]) << i;
  }
  EXPECT_LT(elapsed, chrono::seconds{3});
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    auto level_out = new Log::LevelFilter{chan_out, Log::Level::detail};
    Log::logger.connect(level_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
