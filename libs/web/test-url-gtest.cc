//==========================================================================
// ObTools::Web: test-url-gtest.cc
//
// GTest test harness for URL parsing/resolution
//
// Copyright (c) 2014 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// Basic URL parsing tests
TEST(URLTest, TestQuickAccessForFullURL)
{
  Web::URL url("http://user:password@server/path/path2?foo=bar&n=2#frag");
  EXPECT_EQ("http", url.get_scheme());
  EXPECT_EQ("server", url.get_host());
  EXPECT_EQ("/path/path2", url.get_path());
  EXPECT_EQ("frag", url.get_fragment());
  EXPECT_EQ("foo=bar&n=2", url.get_query());
  EXPECT_EQ("bar", url.get_query_parameter("foo"));
  EXPECT_EQ("2", url.get_query_parameter("n"));
}

TEST(URLTest, TestXMLSplitForFullURL)
{
  Web::URL url("http://user:password@server/path/path2?foo=bar&n=2#frag");
  XML::Element xml;
  ASSERT_TRUE(url.split(xml));

  EXPECT_EQ("http", xml.get_child("scheme").content);
  EXPECT_EQ("user", xml.get_child("user").content);
  EXPECT_EQ("password", xml.get_child("password").content);
  EXPECT_EQ("server", xml.get_child("host").content);
  EXPECT_EQ("/path/path2", xml.get_child("path").content);
  EXPECT_EQ("frag", xml.get_child("fragment").content);
  EXPECT_EQ("foo=bar&n=2", xml.get_child("query").content);
}

//==========================================================================
// URL resolution tests
TEST(URLTest, TestResolveRelativeURL)
{
  Web::URL base("http://user:password@server/path/path2");
  Web::URL url("file.txt");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://user:password@server/path/file.txt", resolved.str());
}

TEST(URLTest, TestResolveRelativeURLWithQueryAndFragment)
{
  Web::URL base("http://server/path/path2");
  Web::URL url("script.php?foo=bar#frag");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://server/path/script.php?foo=bar#frag", resolved.str());
}

TEST(URLTest, TestResolveAbsoluteURL)
{
  Web::URL base("http://server/path/path2");
  Web::URL url("http://server2/path2");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://server2/path2", resolved.str());
}

TEST(URLTest, TestResolveServerRelativeURL)
{
  Web::URL base("http://server/path/path2");
  Web::URL url("/path2");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://server/path2", resolved.str());
}

TEST(URLTest, TestResolveDotDotRelativeURL)
{
  Web::URL base("http://server/path/path2");
  Web::URL url("../foo");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://server/foo", resolved.str());
}

TEST(URLTest, TestResolveDoubleDotDotRelativeURL)
{
  Web::URL base("http://server/path/path2/path3");
  Web::URL url("../../foo");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://server/foo", resolved.str());
}

TEST(URLTest, TestResolveNonRootedDotDotRelativeURL)
{
  Web::URL base("http://server/path/path2/path3");
  Web::URL url("../foo");
  Web::URL resolved = url.resolve(base);
  ASSERT_EQ("http://server/path/foo", resolved.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
