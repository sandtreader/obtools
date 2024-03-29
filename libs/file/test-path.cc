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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <zconf.h>

namespace {

using namespace std;
using namespace ObTools;

class PathTest: public ::testing::Test
{
public:
  PathTest()
  {
    close(open("no-access", O_CREAT, 0000));
    close(open("read-only", O_CREAT, 0444));
    close(open("read-writeable", O_CREAT, 0644));
  }

  ~PathTest()
  {
    unlink("no-access");
    unlink("read-only");
    unlink("read-writeable");
  }
};

TEST_F(PathTest, TestIsAbsolute)
{
  EXPECT_TRUE(File::Path("/absolute/path/to/something").is_absolute());
  EXPECT_FALSE(File::Path("~/relative/to/home").is_absolute());
  EXPECT_FALSE(File::Path("~relative/to/home").is_absolute());
  EXPECT_FALSE(File::Path("./relative/to/here").is_absolute());
}

TEST_F(PathTest, TestDirName)
{
  EXPECT_EQ("/absolute/path/to/something",
            File::Path("/absolute/path/to/something/").dirname());
  EXPECT_EQ("/absolute/path/to/something",
            File::Path("/absolute/path/to/something/deeper").dirname());
  EXPECT_EQ("~/relative/path/to/home",
            File::Path("~/relative/path/to/home/").dirname());
  EXPECT_EQ("~relative/path/to/home",
            File::Path("~relative/path/to/home/").dirname());
  EXPECT_EQ("./relative/to/here",
            File::Path("./relative/to/here/").dirname());
  EXPECT_EQ("/",
            File::Path("/absolute").dirname());
  EXPECT_EQ(".",
            File::Path("foo").dirname());
}

TEST_F(PathTest, TestLeafName)
{
  EXPECT_EQ("", File::Path("/absolute/path/to/something/").leafname());
  EXPECT_EQ("deeper",
            File::Path("/absolute/path/to/something/deeper").leafname());
  EXPECT_EQ(".", File::Path(".").leafname());
  EXPECT_EQ("local", File::Path("local").leafname());
}

TEST_F(PathTest, TestExtension)
{
  EXPECT_EQ("txt", File::Path("readme.txt").extension());
  EXPECT_EQ("emacs", File::Path("~/.emacs").extension());
  EXPECT_EQ("bashrc", File::Path(".bashrc").extension());
  EXPECT_EQ("txt", File::Path("./n.as.ty./.pa.th/.he.ll.txt").extension());
}

TEST_F(PathTest, TestBasename)
{
  EXPECT_EQ("readme", File::Path("readme.txt").basename());
  EXPECT_EQ("", File::Path("~/.emacs").basename());
  EXPECT_EQ("", File::Path(".bashrc").basename());
  EXPECT_EQ(".he.ll", File::Path("./n.as.ty./.pa.th/.he.ll.txt").basename());
}

TEST_F(PathTest, TestResolve)
{
  EXPECT_EQ("/foo/splat",
            File::Path("/foo/bar").resolve(File::Path("splat")).str());
  EXPECT_EQ("/splat",
            File::Path("/foo/bar").resolve(File::Path("/splat")).str());
  EXPECT_EQ("/splat",
            File::Path("/foo/bar").resolve(File::Path("../splat")).str());
  EXPECT_EQ("../splat",
            File::Path(".").resolve(File::Path("../splat")).str());
  EXPECT_EQ("splat",
            File::Path("foo/bar").resolve(File::Path("../splat")).str());
  EXPECT_EQ("splat",
            File::Path("./foo/bar").resolve(File::Path("../splat")).str());
}

TEST_F(PathTest, TestExists)
{
  EXPECT_TRUE(File::Path(".").exists());
  EXPECT_TRUE(File::Path("./").exists());
  EXPECT_TRUE(File::Path("/").exists());
  EXPECT_FALSE(File::Path("non-existant").exists());
}

TEST_F(PathTest, TestIsDir)
{
  EXPECT_TRUE(File::Path(".").is_dir());
  EXPECT_TRUE(File::Path("./").is_dir());
  EXPECT_TRUE(File::Path("/").is_dir());
  EXPECT_FALSE(File::Path("read-only").is_dir());
  EXPECT_FALSE(File::Path("non-existant").is_dir());
}

TEST_F(PathTest, TestReadable)
{
  EXPECT_TRUE(File::Path(".").readable());
  EXPECT_TRUE(File::Path("./").readable());
  EXPECT_TRUE(File::Path("read-only").readable());
  EXPECT_TRUE(File::Path("/").readable());
  if (geteuid())  // Root can always read
    EXPECT_FALSE(File::Path("no-access").readable());
  EXPECT_FALSE(File::Path("non-existant").readable());
}

TEST_F(PathTest, TestWritable)
{
  EXPECT_TRUE(File::Path(".").writeable());
  EXPECT_TRUE(File::Path("./").writeable());
  EXPECT_TRUE(File::Path("read-writeable").writeable());
  EXPECT_TRUE(File::Path("non-existant").writeable());
  EXPECT_TRUE(File::Path("./non-existant").writeable());
  if (geteuid())  // Root can always write
  {
    EXPECT_FALSE(File::Path("read-only").writeable());
    EXPECT_FALSE(File::Path("no-access").writeable());
  }
  EXPECT_FALSE(File::Path("nowhere/non-existant").writeable());
}

TEST_F(PathTest, TestLength)
{
  int f = open("eight-bytes", O_CREAT | O_WRONLY, 0644);
  write(f, "01234567", 8);
  close(f);
  EXPECT_EQ(8, File::Path("eight-bytes").length());
  EXPECT_EQ(0, File::Path("non-existant").length());
  unlink("eight-bytes");
}

TEST_F(PathTest, TestMode)
{
  EXPECT_EQ(0100000, File::Path("no-access").mode());
  EXPECT_EQ(0000000, File::Path("non-existant").mode());
  EXPECT_EQ(0100444, File::Path("read-only").mode());
  EXPECT_EQ(0100644, File::Path("read-writeable").mode());
}

TEST_F(PathTest, TestGetNameFromUserID)
{
  EXPECT_EQ("root", File::Path::user_id_to_name(0));
  EXPECT_EQ("nobody", File::Path::user_id_to_name(65534));
}

TEST_F(PathTest, TestGetNameFromGroupID)
{
  EXPECT_EQ("root", File::Path::group_id_to_name(0));
  EXPECT_EQ("nogroup", File::Path::group_id_to_name(65534));
}

TEST_F(PathTest, TestGetIDFromUserName)
{
  EXPECT_EQ(0, File::Path::user_name_to_id("root"));
  EXPECT_EQ(65534, File::Path::user_name_to_id("nobody"));
}

TEST_F(PathTest, TestGetIDFromGroupName)
{
  EXPECT_EQ(0, File::Path::group_name_to_id("root"));
  EXPECT_EQ(65534, File::Path::group_name_to_id("nogroup"));
}

TEST_F(PathTest, TestTouchCreate)
{
  string p = "touch-create";

  File::Path path(p);

  ASSERT_FALSE(path.exists());

  int touchsuccess = path.touch();
  int new_length = path.length();
  EXPECT_EQ(0, new_length);
  EXPECT_EQ(true, touchsuccess);
  unlink(p.c_str());
}

TEST_F(PathTest, TestTouch)
{
  string p = "touch";

  int f = open(p.c_str(), O_CREAT | O_WRONLY, 0644);
  write(f, "0123456789", 10);
  close(f);

  File::Path path(p);

  time_t old_last_modified = path.last_modified();
  this_thread::sleep_for(chrono::seconds(1));
  int old_length = path.length();
  int touchsuccess = path.touch();
  time_t new_last_modified = path.last_modified();
  int new_length = path.length();

  EXPECT_EQ(old_length, new_length);
  EXPECT_EQ(10, new_length);
  EXPECT_EQ(true, touchsuccess);
  EXPECT_GT(new_last_modified, old_last_modified);
  unlink(p.c_str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
