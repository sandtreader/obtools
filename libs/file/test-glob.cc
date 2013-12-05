//==========================================================================
// ObTools::File: test-glob.cc
//
// Test harness for file path library glob
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

const string test_dir("../test.dump");

class GlobTest: public ::testing::Test
{
  virtual void SetUp()
  {
    File::Directory dir(test_dir);
    // Ensure test dir does not exist
    dir.erase();
    // Create test dir
    dir.ensure(true);

    File::Path patha(test_dir + "/a");
    patha.touch();
    File::Path pathb(test_dir + "/b");
    pathb.touch();
    File::Path pathc(test_dir + "/c");
    pathc.touch();
  }

  virtual void TearDown()
  {
    // Remove test dir
    File::Directory dir(test_dir);
    dir.erase();
  }
};

TEST_F(GlobTest, TestListing)
{
  vector<string> expected;
  expected.push_back("../test.dump/a");
  expected.push_back("../test.dump/b");
  expected.push_back("../test.dump/c");

  string pattern(test_dir + "/*");
  File::Glob glob(pattern);

  vector<string> actual;
  actual.insert(actual.end(), glob.begin(), glob.end());

  ASSERT_EQ(expected, actual);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
