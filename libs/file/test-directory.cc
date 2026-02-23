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

TEST(DirectoryTest, TestCombineConstructor)
{
  EXPECT_EQ("foo/bar", File::Directory("foo", "bar").str());
}

TEST(DirectoryTest, TestResolve)
{
  EXPECT_EQ("/foo/bar/splat",
            File::Directory("/foo/bar").resolve(File::Path("splat")).str());
  EXPECT_EQ("/splat",
            File::Directory("/foo/bar").resolve(File::Path("/splat")).str());
  EXPECT_EQ("/foo/splat",
            File::Directory("/foo/bar").resolve(File::Path("../splat")).str());
  EXPECT_EQ("../splat",
            File::Directory(".").resolve(File::Path("../splat")).str());
  EXPECT_EQ("splat",
            File::Directory("foo").resolve(File::Path("../splat")).str());
  EXPECT_EQ("splat",
            File::Directory("./foo").resolve(File::Path("../splat")).str());
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

//--------------------------------------------------------------------------
// Directory::ensure on already-existing directory
TEST(DirectoryTest, TestEnsureAlreadyExists)
{
  auto d = File::Directory{"/tmp/obtools-test-ensure-exists"};
  ASSERT_TRUE(d.ensure(true));
  // Call ensure again - mkdir fails but directory exists
  EXPECT_TRUE(d.ensure(false));
  d.erase();
}

//--------------------------------------------------------------------------
// Directory::exists()
TEST(DirectoryTest, TestDirectoryExists)
{
  EXPECT_TRUE(File::Directory("/tmp").exists());
  EXPECT_FALSE(File::Directory("/nonexistent_xyz_99").exists());
}

//--------------------------------------------------------------------------
// Directory::empty()
TEST(DirectoryTest, TestDirectoryEmpty)
{
  auto d = File::Directory{"/tmp/obtools-test-empty-dir"};
  d.ensure(true);
  EXPECT_TRUE(d.empty());

  // Add a file to make it non-empty
  File::Path p(d, "file.txt");
  p.touch();
  EXPECT_FALSE(d.empty());

  d.erase();
}

//--------------------------------------------------------------------------
// Directory::extend(const Path&)
TEST(DirectoryTest, TestDirectoryExtendWithPath)
{
  File::Directory d("/foo");
  d.extend(File::Path("bar"));
  EXPECT_EQ("/foo/bar", d.str());

  File::Directory d2("/foo");
  d2.extend(File::Path("/bar"));
  EXPECT_EQ("/foo/bar", d2.str());
}

//--------------------------------------------------------------------------
// Glob::erase()
TEST(DirectoryTest, TestGlobErase)
{
  auto d = File::Directory{"/tmp/obtools-test-glob-erase"};
  d.ensure(true);

  File::Path(d, "a.tmp").touch();
  File::Path(d, "b.tmp").touch();
  ASSERT_TRUE(File::Path(d, "a.tmp").exists());
  ASSERT_TRUE(File::Path(d, "b.tmp").exists());

  File::Glob glob("/tmp/obtools-test-glob-erase/*.tmp");
  EXPECT_TRUE(glob.erase());

  EXPECT_FALSE(File::Path(d, "a.tmp").exists());
  EXPECT_FALSE(File::Path(d, "b.tmp").exists());
  d.erase();
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
