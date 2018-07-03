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

TEST(DirectoryTest, TestResolve)
{
  EXPECT_EQ("/foo/bar/splat",
            File::Directory("/foo/bar").resolve(File::Path("splat")).str());
  EXPECT_EQ("/splat",
            File::Directory("/foo/bar").resolve(File::Path("/splat")).str());
  EXPECT_EQ("/foo/splat",
            File::Directory("/foo/bar").resolve(File::Path("../splat")).str());
}

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

TEST(DirectoryTest, TestRecursiveDirectoryInspect)
{
  auto test_dir = File::Directory{"./test-gubbins"};
  test_dir.ensure(true);
  File::Path foo1(test_dir, "foo1.txt");
  foo1.touch();
  ASSERT_TRUE(foo1.exists());
  File::Path bar1(test_dir, "bar1.not");
  bar1.touch();
  ASSERT_TRUE(bar1.exists());

  auto dir2 = File::Directory{test_dir}.extend("foo");
  dir2.ensure(true);
  File::Path foo2(dir2, ".foo2.txt");
  foo2.touch();
  ASSERT_TRUE(foo2.exists());
  File::Path bar2(dir2, "bar2.not");
  bar2.touch();
  ASSERT_TRUE(bar2.exists());

  list<File::Path> paths;
  test_dir.inspect_recursive(paths, "*.txt", true);
  EXPECT_EQ(2, paths.size());

  test_dir.erase();
  EXPECT_FALSE(test_dir.exists());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
