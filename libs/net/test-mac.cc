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
  // Look in ip neigh show for someone we know
  FILE *f = popen("/bin/ip neigh show | head -n 1 | cut -d\" \" -f 1", "r");
  ASSERT_TRUE(f != NULL) << "Can't run `ip neigh show`";

  char line[81];
  fgets(line, sizeof(line)-1, f);
  ASSERT_GT(strlen(line), 0);
  line[strlen(line)-1] = 0; // chomp NL
  fclose(f);

  Net::TCPSocket socket;
  string mac = socket.get_mac(Net::IPAddress(line));
  ASSERT_NE(mac, "");
  EXPECT_EQ(mac.size(), 17);

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
