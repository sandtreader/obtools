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

//--------------------------------------------------------------------------
// Pure string/computation tests - no filesystem
TEST_F(PathTest, TestDir)
{
  File::Directory d = File::Path("/foo/bar/baz").dir();
  EXPECT_EQ("/foo/bar", d.str());
}

TEST_F(PathTest, TestExtendWithPath)
{
  File::Path p("/foo");
  p.extend(File::Path("bar"));
  EXPECT_EQ("/foo/bar", p.str());

  File::Path p2("/foo");
  p2.extend(File::Path("/bar"));
  EXPECT_EQ("/foo/bar", p2.str());
}

TEST_F(PathTest, TestExpandOnLinux)
{
  // On Linux, expand() is a no-op
  File::Path p("/some/path");
  EXPECT_EQ("/some/path", p.expand().str());
}

TEST_F(PathTest, TestFixSlashesOnLinux)
{
  // On Linux, fix_slashes() is a no-op
  File::Path p("/some/path");
  p.fix_slashes();
  EXPECT_EQ("/some/path", p.str());
}

TEST_F(PathTest, TestItoo)
{
  EXPECT_EQ("755", File::Path::itoo(0755));
  EXPECT_EQ("644", File::Path::itoo(0644));
  EXPECT_EQ("0", File::Path::itoo(0));
}

TEST_F(PathTest, TestOtoi)
{
  EXPECT_EQ(0755, File::Path::otoi("755"));
  EXPECT_EQ(0644, File::Path::otoi("644"));
  EXPECT_EQ(0, File::Path::otoi("0"));
}

TEST_F(PathTest, TestSanitiseLeaf)
{
  EXPECT_EQ("hello_world_", File::Path::sanitise_leaf("hello world!"));
  EXPECT_EQ("safe.name_txt", File::Path::sanitise_leaf("safe.name_txt"));
  EXPECT_EQ("a-b+c.d_e", File::Path::sanitise_leaf("a-b+c.d_e"));
  EXPECT_EQ("___", File::Path::sanitise_leaf("@#$"));
}

TEST_F(PathTest, TestOstreamOperator)
{
  File::Path p("/foo/bar");
  ostringstream oss;
  oss << p;
  EXPECT_EQ("/foo/bar", oss.str());
}

//--------------------------------------------------------------------------
// Filesystem tests using /tmp

TEST_F(PathTest, TestRealpathSuccess)
{
  File::Path p("/tmp");
  File::Path rp = p.realpath();
  EXPECT_FALSE(rp.str().empty());
}

TEST_F(PathTest, TestRealpathFailure)
{
  File::Path p("/nonexistent_path_xyz_123");
  File::Path rp = p.realpath();
  EXPECT_TRUE(!rp);
}

TEST_F(PathTest, TestSetLastModified)
{
  string tmp = "/tmp/obtools-test-mtime";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  time_t target = 1000000;
  EXPECT_TRUE(p.set_last_modified(target));
  EXPECT_EQ(target, p.last_modified());
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestSetMode)
{
  string tmp = "/tmp/obtools-test-mode";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  EXPECT_TRUE(p.set_mode(0755));
  EXPECT_EQ(0100755, p.mode());
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestOwnerAndGroup)
{
  string tmp = "/tmp/obtools-test-owner";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  // Owner and group should be valid (non-zero or zero for root)
  uid_t o = p.owner();
  gid_t g = p.group();
  // Just verify they return without error (value depends on user)
  (void)o;
  (void)g;
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestSetOwnershipByUidGid)
{
  // set_ownership with uid/gid - we can at least test with current owner
  string tmp = "/tmp/obtools-test-chown";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  EXPECT_TRUE(p.set_ownership(p.owner(), p.group()));
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestSetOwnershipByValidNames)
{
  // set_ownership with valid user/group string names
  string tmp = "/tmp/obtools-test-chown-names";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  // Use current user/group names
  string uname = File::Path::user_id_to_name(p.owner());
  string gname = File::Path::group_id_to_name(p.group());
  EXPECT_TRUE(p.set_ownership(uname, gname));
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestSetOwnershipByNameInvalid)
{
  // Bad user/group names should return false
  string tmp = "/tmp/obtools-test-chown-bad";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  EXPECT_FALSE(p.set_ownership("nonexistent_user_xyz", "nonexistent_group_xyz"));
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestRenameFile)
{
  string src = "/tmp/obtools-test-rename-src";
  string dst = "/tmp/obtools-test-rename-dst";
  File::Path sp(src);
  sp.touch();
  ASSERT_TRUE(sp.exists());

  EXPECT_TRUE(sp.rename(File::Path(dst)));
  EXPECT_FALSE(sp.exists());
  EXPECT_TRUE(File::Path(dst).exists());
  unlink(dst.c_str());
}

TEST_F(PathTest, TestWriteAllByteVector)
{
  string tmp = "/tmp/obtools-test-write-vec";
  File::Path p(tmp);
  vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o'};
  EXPECT_EQ("", p.write_all(data));

  string contents;
  EXPECT_TRUE(p.read_all(contents));
  EXPECT_EQ("Hello", contents);
  unlink(tmp.c_str());
}

TEST_F(PathTest, TestEraseRegularFile)
{
  string tmp = "/tmp/obtools-test-erase-file";
  File::Path p(tmp);
  p.touch();
  ASSERT_TRUE(p.exists());

  EXPECT_TRUE(p.erase());
  EXPECT_FALSE(p.exists());
}

TEST_F(PathTest, TestReadAllNonexistent)
{
  File::Path p("/tmp/obtools-nonexistent-xyz-123");
  string s;
  EXPECT_FALSE(p.read_all(s));
  // s contains error message
  EXPECT_FALSE(s.empty());
}

TEST_F(PathTest, TestGetNameFromInvalidUserID)
{
  EXPECT_EQ("UNKNOWN", File::Path::user_id_to_name(99999999));
}

TEST_F(PathTest, TestGetNameFromInvalidGroupID)
{
  EXPECT_EQ("UNKNOWN", File::Path::group_id_to_name(99999999));
}

TEST_F(PathTest, TestGetIDFromInvalidUserName)
{
  EXPECT_EQ(-1, File::Path::user_name_to_id("nonexistent_user_xyz_99"));
}

TEST_F(PathTest, TestGetIDFromInvalidGroupName)
{
  EXPECT_EQ(-1, File::Path::group_name_to_id("nonexistent_group_xyz_99"));
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
