//==========================================================================
// ObTools::Net: test-mac.cc
//
// Test MAC address fetching
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-net.h"
#include <iostream>
#include <limits.h>

using namespace std;
using namespace ObTools;

TEST(MACTests, TestMACValid)
{
  Net::TCPSocket socket;
  string mac = socket.get_mac(Net::IPAddress("testhost"));
  ASSERT_NE(mac, "");

  // Check it has 5 colons and nothing outside hex range
  int colons = 0;
  for(string::size_type i=0; i<mac.size(); i++)
  {
    char c = mac[i];
    if (c == ':') colons++;
    else EXPECT_TRUE((c>='0' && c<='9') || (c>='A' && c<='F'))
           << "Invalid character: " << c;
  }

  EXPECT_EQ(5, colons);
}

TEST(MACTests, TestBroadcastHasNoMAC)
{
  Net::TCPSocket socket;
  string mac = socket.get_mac(Net::IPAddress("0.0.0.0"));
  ASSERT_EQ(mac, "");
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
