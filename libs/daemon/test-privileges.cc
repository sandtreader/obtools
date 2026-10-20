//==========================================================================
// ObTools::Daemon: test-privileges.cc
//
// GTest harness for changing user and group when dropping privileges
// Tests which actually change user need to run as root, and are skipped
// otherwise.  They run in a forked child so the test process keeps root.
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-daemon.h"
#include <gtest/gtest.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <vector>

namespace {

using namespace std;
using namespace ObTools;

// Run a check in a forked child, returning its exit code (0 = passed)
template<class F> int in_child(F f)
{
  auto pid = fork();
  if (!pid) _exit(f());
  int status = 0;
  waitpid(pid, &status, 0);
  return WIFEXITED(status) ? WEXITSTATUS(status) : 99;
}

vector<gid_t> get_groups()
{
  vector<gid_t> groups(getgroups(0, nullptr));
  groups.resize(getgroups(groups.size(), groups.data()));
  return groups;
}

bool has_group(gid_t gid)
{
  auto groups = get_groups();
  return find(groups.begin(), groups.end(), gid) != groups.end();
}

TEST(PrivilegesTest, TestUnknownUserFails)
{
  EXPECT_FALSE(Daemon::change_user_and_group("no-such-user-xyzzy", "")
               .empty());
}

TEST(PrivilegesTest, TestUnknownGroupFails)
{
  EXPECT_FALSE(Daemon::change_user_and_group("", "no-such-group-xyzzy")
               .empty());
}

TEST(PrivilegesTest, TestUserGetsOwnGroupsNotRoots)
{
  if (getuid()) GTEST_SKIP() << "needs root";
  auto pw = getpwnam("nobody");
  ASSERT_NE(nullptr, pw);
  auto uid = pw->pw_uid;
  auto gid = pw->pw_gid;

  EXPECT_EQ(0, in_child([=]() {
    // Start with root's supplementary groups, as a daemon started by root
    gid_t root_groups[] = { 0, 4 };
    if (setgroups(2, root_groups)) return 1;

    if (!Daemon::change_user_and_group("nobody", "").empty()) return 2;
    if (getuid() != uid || geteuid() != uid) return 3;

    // No group given - user's primary group, not root's
    if (getgid() != gid || getegid() != gid) return 4;

    // Root's supplementary groups dropped
    if (has_group(0) || (gid != 4 && has_group(4))) return 5;
    return 0;
  }));
}

TEST(PrivilegesTest, TestUserAndGroup)
{
  if (getuid()) GTEST_SKIP() << "needs root";
  auto pw = getpwnam("nobody");
  ASSERT_NE(nullptr, pw);
  auto gr = getgrnam("daemon");
  ASSERT_NE(nullptr, gr);
  auto uid = pw->pw_uid;
  auto gid = gr->gr_gid;

  EXPECT_EQ(0, in_child([=]() {
    gid_t root_groups[] = { 0 };
    if (setgroups(1, root_groups)) return 1;

    if (!Daemon::change_user_and_group("nobody", "daemon").empty()) return 2;
    if (getuid() != uid) return 3;
    if (getgid() != gid || getegid() != gid) return 4;
    if (has_group(0)) return 5;
    return 0;
  }));
}

TEST(PrivilegesTest, TestGroupOnly)
{
  if (getuid()) GTEST_SKIP() << "needs root";
  auto gr = getgrnam("daemon");
  ASSERT_NE(nullptr, gr);
  auto gid = gr->gr_gid;

  EXPECT_EQ(0, in_child([=]() {
    gid_t root_groups[] = { 0, 4 };
    if (setgroups(2, root_groups)) return 1;

    if (!Daemon::change_user_and_group("", "daemon").empty()) return 2;
    if (getuid() != 0) return 3;  // Still root
    if (getgid() != gid) return 4;

    // Only the group given
    auto groups = get_groups();
    if (groups.size() > 1 || (groups.size() == 1 && groups[0] != gid))
      return 5;
    return 0;
  }));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
