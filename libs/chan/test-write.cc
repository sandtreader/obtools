//==========================================================================
// ObTools::Channel: test-write.cc
//
// Test harness for writing
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-chan.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(WriteTest, TestChoppingLongStringIntoFixedField)
{
  string data;
  Channel::StringWriter sw(data);
  sw.write_fixed("Wombats are go!", 6);
  EXPECT_EQ("Wombat", data);
}

TEST(WriteTest, TestPaddingShortStringIntoFixedField)
{
  string data;
  Channel::StringWriter sw(data);
  sw.write_fixed("Wom", 6, ' ');
  EXPECT_EQ("Wom   ", data);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
