//==========================================================================
// ObTools::FDB: test-fdb.cc
//
// Test harness for FDB C++ binding
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include <gtest/gtest.h>
#include "ot-fdb.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(FDBTest, TestFoo)
{
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
