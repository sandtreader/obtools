//==========================================================================
// ObTools::Tube: test-tube.cc
//
// Test harness for Tube functions
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-tube.h"

using namespace std;
using namespace ObTools;

TEST(TagTests, TestTagRoundTrip)
{
  Tube::tag_t ABCD_tag = Tube::string_to_tag("ABCD");
  ASSERT_EQ(ABCD_tag, 0x41424344);

  string ABCD_recovered_tag = Tube::tag_to_string(ABCD_tag);
  ASSERT_EQ(ABCD_recovered_tag, "ABCD");
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



