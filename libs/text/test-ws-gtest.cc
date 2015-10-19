//==========================================================================
// ObTools::Text: test-ws-gtest.cc
//
// GTest harness for text library whitespace functions
//
// Copyright (c) 2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(WSTest, TestRemoveSpace)
{
  ASSERT_EQ("foo", Text::remove_space(" \tf\n\r  oo\r\n "));
}


} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
