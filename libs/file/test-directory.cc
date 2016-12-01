//==========================================================================
// ObTools::File: test-directory.cc
//
// Test harness for File::Directory
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <gtest/gtest.h>
#include <thread>

namespace {

using namespace std;
using namespace ObTools;

TEST(DirectoryTest, TestRecursiveDirectoryDeletion)
{
  auto test_dir = File::Directory{"./test-gubbins"};
  test_dir.ensure(true);
  auto dir2 = File::Directory{test_dir}.extend("foo/bar");
  dir2.ensure(true);
  auto dir3 = File::Directory{test_dir}.extend("baz");
  dir3.ensure(true);
  test_dir.erase();
  EXPECT_FALSE(test_dir.exists());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
