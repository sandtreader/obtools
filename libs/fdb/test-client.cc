//==========================================================================
// ObTools::FDB: test-client.cc
//
// Test harness for FDB C++ binding, overall client
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include <gtest/gtest.h>
#include "ot-fdb.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(FDBClientTest, TestClientStartsAndStops)
{
  FDB::Client client;
  ASSERT_TRUE(client.start());
  sleep(3);
  client.stop();
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_err = new Log::StreamChannel{&cerr};
    Log::logger.connect(chan_err);
  }

  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
