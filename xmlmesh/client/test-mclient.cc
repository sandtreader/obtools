//==========================================================================
// ObTools::XMLMesh: test-mclient.cc
//
// Test harness for XMLMesh multi-threaded client
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"
#include <unistd.h>

using namespace std;
using namespace ObTools;

TEST(MClientTests, TestMClientExitsReasonablyQuickly)
{
  const auto host = "localhost";
  int port = XMLMesh::OTMP::DEFAULT_PORT;

  Net::IPAddress addr(host);
  ASSERT_FALSE(!addr);
  Net::EndPoint server(addr, port);

  auto client = new XMLMesh::OTMPMultiClient(server);

  Time::Stamp start = Time::Stamp::now();
  delete client;
  Time::Duration taken = Time::Stamp::now()-start;
  ASSERT_LE(taken.seconds(), 1.0);
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y [%*L]: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DETAIL, tsfilter);
  Log::logger.connect(level_out);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}






