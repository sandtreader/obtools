//==========================================================================
// ObTools::File: test-path-gtest.cc
//
// Test harness for file path library
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <iostream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(PathTest, TestTouchCreate)
{
  string p = "./test-path-gtest.gptcreate";

  File::Path path(p);

  if ( path.exists() )
    unlink(p.data());
  
  int touchsuccess = path.touch();
  int new_length = path.length(); 
  ASSERT_EQ(0, new_length);
  ASSERT_EQ(true, touchsuccess);
}

TEST(PathTest, TestTouch)
{
  string p = "./test-path-gtest.gpt";
  
  File::Path path(p);

  time_t old_last_modified = path.last_modified();
  int old_length = path.length();
  int touchsuccess = path.touch();
  time_t new_last_modified = path.last_modified();
  int new_length = path.length();
 
  ASSERT_EQ(old_length, new_length);
  ASSERT_EQ(10, new_length);
  ASSERT_EQ(true, touchsuccess);
  ASSERT_GT(new_last_modified, old_last_modified);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
