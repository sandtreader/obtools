//==========================================================================
// ObTools::Web: test-cookies.cc
//
// Test harness for cookie handling
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// Basic cookie parsing tests
TEST(CookieJarTest, TestBasicCookie)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_TRUE(!cookie.expires);
  EXPECT_TRUE(!cookie.max_age.seconds());
  EXPECT_EQ(cookie.domain, "");
  EXPECT_EQ(cookie.path, "");
  EXPECT_FALSE(cookie.secure);
  EXPECT_FALSE(cookie.http_only);
}

TEST(CookieJarTest, TestQuotedCookie)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=\"foo\""));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
}

TEST(CookieJarTest, TestBlankCookie)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test="));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "");
}

TEST(CookieJarTest, TestBrokenCookies)
{
  Web::Cookie cookie;
  EXPECT_FALSE(cookie.read_from(""));
  EXPECT_FALSE(cookie.read_from("test"));
  EXPECT_FALSE(cookie.read_from("=foo"));
}

TEST(CookieJarTest, TestCookieWithDomainAndPath)
{
  Web::Cookie cookie;
  // Note second ; has no space, illegal but we are liberal
  ASSERT_TRUE(cookie.read_from("test=foo; domain=obtools.com;path=/"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_EQ(cookie.domain, "obtools.com");
  EXPECT_EQ(cookie.path, "/");
}

TEST(CookieJarTest, TestCookieWithExpires)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo; Expires=Thu, 15 Nov 2012 10:52:48 GMT"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_EQ(cookie.expires.iso(), "2012-11-15T10:52:48Z");
}

TEST(CookieJarTest, TestCookieWithMaxAge)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo; Max-Age=3600"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_EQ(cookie.max_age.seconds(), 3600.0);
}

TEST(CookieJarTest, TestCookieWithFlags)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo; Secure; HttpOnly"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_TRUE(cookie.secure);
  EXPECT_TRUE(cookie.http_only);
}

//==========================================================================
// Cookie jar tests
TEST(CookieJarTest, TestBasicCookieRoundTrip)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.com/foo");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "foo");
}

TEST(CookieJarTest, TestCookieNotAcceptedFromDifferentDomain)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://badguys.ru");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "paypal.com",
                      Time::Stamp::now()+Time::Duration("1 day"));
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 0);
}

TEST(CookieJarTest, TestCookieNotSentToDifferentDomain)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://paypal.com");
  Web::URL url2("http://badguys.ru");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "paypal.com",
                      Time::Stamp::now()+Time::Duration("1 day"));
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url2);
  jar.add_cookies_to(request);

  ASSERT_FALSE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("secret"), "");
}

// Same as before but using exact origin map, no domain
TEST(CookieJarTest, TestCookieNotSentToDifferentOrigin)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://paypal.com");
  Web::URL url2("http://host.paypal.com");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password");
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url2);
  jar.add_cookies_to(request);

  ASSERT_FALSE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("secret"), "");
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
