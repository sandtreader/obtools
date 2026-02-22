//==========================================================================
// ObTools::Misc: test-prop.cc
//
// Test harness for propertylist functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

TEST(PropertyListTests, TestCommaDelimited)
{
  Misc::PropertyList pl("a1=99, a2 =   no   spaces   , a3=, a4, a5=\"quoted, see! \"  , a6=got=equals, a7=\"unclosed, a8=xxx");

  EXPECT_EQ(pl.size(), 7);
  EXPECT_EQ(pl["a1"], "99");
  EXPECT_EQ(pl["a2"], "no spaces");
  EXPECT_EQ(pl["a3"], "");
  EXPECT_FALSE(pl.has("a4"));
  EXPECT_EQ(pl["a5"], "quoted, see! ");
  EXPECT_EQ(pl["a6"], "got=equals");
  EXPECT_EQ(pl["a7"], "unclosed");
  EXPECT_EQ(pl["a8"], "xxx");

  EXPECT_EQ(pl.str(), "a1=99,a2=no spaces,a3=,a5=\"quoted, see! \",a6=got=equals,a7=unclosed,a8=xxx");
}

TEST(PropertyListTests, TestValues)
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "!!!");
  pl.add("bar", "two");
  pl.add("XXX", "???");
  pl.add("42", 42);
  pl.add_bool("true", true);

  pl.erase("XXX");

  // Basic fetch tests
  EXPECT_EQ(pl.size(), 4);
  EXPECT_EQ(pl["foo"], "one");
  EXPECT_EQ(pl["bar"], "two");
  EXPECT_EQ(pl["XXX"], "");
  EXPECT_EQ(pl.size(), 4);
  EXPECT_FALSE(pl.has("XXX"));
  EXPECT_EQ(pl.get_int("42"), 42);
  EXPECT_TRUE(pl.get_bool("true"));
}

TEST(PropertyListTests, TestInterpolation)
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "two");

  // Interpolation tests
  EXPECT_EQ(pl.interpolate("$foo"), "one");
  EXPECT_EQ(pl.interpolate("$bar"), "two");
  EXPECT_EQ(pl.interpolate("$foo$bar"), "onetwo");
  EXPECT_EQ(pl.interpolate("$foo;s"), "ones");
  EXPECT_EQ(pl.interpolate("$$$foo$$$bar;"), "$one$two");
  EXPECT_EQ(pl.interpolate("$not"), "$not");
  EXPECT_EQ(pl.interpolate("$not;"), "$not;");
}

TEST(PropertyListTests, TestFromEnvironment)
{
  Misc::PropertyList pl;
  pl.fill_from_environment();
  EXPECT_FALSE(pl.empty());
  EXPECT_TRUE(pl.has("PWD"));
  EXPECT_TRUE(pl.has("PATH"));
}

TEST(PropertyListTests, TestGetReal)
{
  Misc::PropertyList pl;
  pl.add("pi", "3.14159");
  EXPECT_DOUBLE_EQ(3.14159, pl.get_real("pi"));
  EXPECT_DOUBLE_EQ(0.0, pl.get_real("missing"));
  EXPECT_DOUBLE_EQ(9.9, pl.get_real("missing", 9.9));
}

TEST(PropertyListTests, TestDump)
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "two");
  ostringstream oss;
  pl.dump(oss);
  string output = oss.str();
  EXPECT_NE(string::npos, output.find("foo"));
  EXPECT_NE(string::npos, output.find("one"));
}

TEST(PropertyListTests, TestDumpWithCustomPrefixSeparator)
{
  Misc::PropertyList pl;
  pl.add("x", "1");
  ostringstream oss;
  pl.dump(oss, ">> ", ": ");
  string output = oss.str();
  EXPECT_NE(string::npos, output.find(">> x: 1"));
}

TEST(PropertyListTests, TestStreamOperator)
{
  Misc::PropertyList pl;
  pl.add("k", "v");
  ostringstream oss;
  oss << pl;
  EXPECT_NE(string::npos, oss.str().find("k"));
}

TEST(PropertyListTests, TestCopyFromMap)
{
  map<string, string> m;
  m["a"] = "1";
  m["b"] = "2";
  Misc::PropertyList pl(m);
  EXPECT_EQ("1", pl["a"]);
  EXPECT_EQ("2", pl["b"]);
}

TEST(PropertyListTests, TestAssignFromMap)
{
  map<string, string> m;
  m["x"] = "y";
  Misc::PropertyList pl;
  pl = m;
  EXPECT_EQ("y", pl["x"]);
}

TEST(PropertyListTests, TestAddUint64)
{
  Misc::PropertyList pl;
  pl.add("big", uint64_t{12345678901234567890ULL});
  EXPECT_EQ("12345678901234567890", pl["big"]);
}

TEST(PropertyListTests, TestInterpolationEscapedSemicolon)
{
  Misc::PropertyList pl;
  pl.add("x", "val");
  // $; should produce a literal semicolon
  EXPECT_EQ("val;", pl.interpolate("$x$;"));
}

TEST(PropertyListTests, TestInterpolationNonAlphanumAfterDollar)
{
  Misc::PropertyList pl;
  // $! — the ! is not alphanum or $ or ; so default case reinserts "$!"
  EXPECT_EQ("$!", pl.interpolate("$!"));
  EXPECT_EQ("hello$!world", pl.interpolate("hello$!world"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

