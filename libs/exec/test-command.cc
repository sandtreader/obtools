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

int main(int argc, char **argv)
{
  Log::StreamChannel chan_out(cout);
  Log::LevelFilter level_out(Log::LEVEL_DETAIL, chan_out);
  if (argc > 1 && string(argv[1]) == "-v")
    Log::logger.connect(level_out);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
