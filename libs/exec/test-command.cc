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

TEST(Command, TestNothing)
{

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
