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
  ASSERT_TRUE(cookie.read_from("test=foo; domain=obtools.org;path=/"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_EQ(cookie.domain, "obtools.org");
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

TEST(CookieJarTest, TestCookieWithMaxAgeInFuture)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo; Max-Age=3600"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_GE(cookie.expires, Time::Stamp::now()+Time::Duration(3599));
  EXPECT_LE(cookie.expires, Time::Stamp::now()+Time::Duration(3601));
}

TEST(CookieJarTest, TestCookieWithMaxAgeZero)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo; Max-Age=0"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_EQ(cookie.expires, Time::Stamp(1));
}

TEST(CookieJarTest, TestCookieWithMaxAgeNegative)
{
  Web::Cookie cookie;
  ASSERT_TRUE(cookie.read_from("test=foo; Max-Age=-999"));
  EXPECT_EQ(cookie.name, "test");
  EXPECT_EQ(cookie.value, "foo");
  EXPECT_EQ(cookie.expires, Time::Stamp(1));
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
  Web::URL url("http://obtools.org/foo");

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

TEST(CookieJarTest, TestCookieRoundTripFromSubdomain)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://www.obtools.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "obtools.org");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "foo");
}

TEST(CookieJarTest, TestCookieRoundTripFromExplicitPathPrefixTopLevel)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "obtools.org");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "foo");
}

TEST(CookieJarTest, TestCookieRoundTripFromExplicitPathPrefixLowerLevel)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo/bar");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/foo", "obtools.org");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

}

TEST(CookieJarTest, TestSecureCookieRoundTripToSecureHost)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url("https://paypal.com");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "paypal.com",
                      Time::Stamp::now()+Time::Duration("1 day"), true);
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("secret"), "password");
}

TEST(CookieJarTest, TestHTTPOnlyCookieRoundTripToHTTP)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url("http://obtools.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "", Time::Stamp(), false, true);
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "foo");
}

TEST(CookieJarTest, TestHTTPOnlyCookieRoundTripToHTTPS)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url("https://obtools.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "", Time::Stamp(), false, true);
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "foo");
}

TEST(CookieJarTest,
     TestCookieRoundTripFromExplicitPathPrefixLowerLevelWithSlash)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo/bar");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/foo/", "obtools.org");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "foo");
}

TEST(CookieJarTest, TestCookieNotAcceptedFromNonDotPrefixDomain)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://badobtools.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "obtools.org",
                      Time::Stamp::now()+Time::Duration("1 day"));
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 0);
}

TEST(CookieJarTest, TestHTTPOnlyCookieAcceptedFromHTTP)
{
  Web::CookieJar jar;
  Web::URL url("http://obtools.org");
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "", Time::Stamp(), false, true);
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);
}

TEST(CookieJarTest, TestHTTPOnlyCookieAcceptedFromHTTPS)
{
  Web::CookieJar jar;
  Web::URL url("https://obtools.org");
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "", Time::Stamp(), false, true);
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);
}

TEST(CookieJarTest, TestHTTPOnlyCookieNotAcceptedFromNonHTTP)
{
  Web::CookieJar jar;
  Web::URL url("ftp://nothttp.org");
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "/", "", Time::Stamp(), false, true);
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 0);
}

TEST(CookieJarTest, TestCookieReplacement)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo");

  // Create response and feed to jar
  Web::HTTPMessage response1(200, "OK");
  response1.set_cookie("test", "foo");
  jar.take_cookies_from(response1, url);
  ASSERT_EQ(jar.count(), 1);

  Web::HTTPMessage response2(200, "OK");
  response2.set_cookie("test", "bar");
  jar.take_cookies_from(response2, url);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_TRUE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "bar");
}

TEST(CookieJarTest, TestCookieDeletion)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo");

  // Create response and feed to jar
  Web::HTTPMessage response1(200, "OK");
  response1.set_cookie("test", "foo");
  jar.take_cookies_from(response1, url);
  ASSERT_EQ(jar.count(), 1);

  Web::HTTPMessage response2(200, "OK");
  response2.set_cookie("test", "", "", "", Time::Stamp(1));
  jar.take_cookies_from(response2, url);
  jar.prune();
  ASSERT_EQ(jar.count(), 0);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url);
  jar.add_cookies_to(request);

  ASSERT_FALSE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("test"), "");
}

TEST(CookieJarTest, TestSessionCookiesDeletedAtEndOfSession)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);
  jar.prune(true);
  ASSERT_EQ(jar.count(), 0);
}

TEST(CookieJarTest, TestSessionCookiesNotDeletedInRegularPrune)
{
  Web::CookieJar jar;

  // URL
  Web::URL url("http://obtools.org/foo");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo");
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 1);
  jar.prune(false);
  ASSERT_EQ(jar.count(), 1);
}

TEST(CookieJarTest, TestCookieNotAcceptedFromDifferentDomain)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url("http://badguys.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "paypal.com",
                      Time::Stamp::now()+Time::Duration("1 day"));
  jar.take_cookies_from(response, url);
  ASSERT_EQ(jar.count(), 0);
}

TEST(CookieJarTest, TestCookieNotSentToDifferentDomain)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://paypal.com");
  Web::URL url2("http://badguys.org");

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

TEST(CookieJarTest, TestCookieNotSentToNonDotPrefixDomain)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://obtools.org");
  Web::URL url2("http://badobtools.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "obtools.org",
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

TEST(CookieJarTest, TestCookieNotSentToDifferentExplicitPath)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://obtools.org/foo");
  Web::URL url2("http://obtools.org/bar");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/foo", "obtools.org",
                      Time::Stamp::now()+Time::Duration("1 day"));
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url2);
  jar.add_cookies_to(request);

  ASSERT_FALSE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("secret"), "");
}

TEST(CookieJarTest, TestCookieNotSentToDifferentImplicitPath)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://obtools.org/foo/");
  Web::URL url2("http://obtools.org/bar/");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "", "obtools.org",
                      Time::Stamp::now()+Time::Duration("1 day"));
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url2);
  jar.add_cookies_to(request);

  ASSERT_FALSE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("secret"), "");
}

TEST(CookieJarTest, TestSecureCookieNotSentToInsecureHost)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("https://paypal.com");
  Web::URL url2("http://paypal.com");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("secret", "password", "/", "paypal.com",
                      Time::Stamp::now()+Time::Duration("1 day"), true);
  jar.take_cookies_from(response, url1);
  ASSERT_EQ(jar.count(), 1);

  // Create request and put cookies back
  Web::HTTPMessage request("GET", url2);
  jar.add_cookies_to(request);

  ASSERT_FALSE(request.headers.has("cookie"));
  ASSERT_EQ(request.get_cookie("secret"), "");
}

TEST(CookieJarTest, TestHTTPOnlyCookieNotSentToNonHTTP)
{
  Web::CookieJar jar;

  // URLs
  Web::URL url1("http://obtools.org");
  Web::URL url2("ftp://obtools.org");

  // Create response and feed to jar
  Web::HTTPMessage response(200, "OK");
  response.set_cookie("test", "foo", "", "", Time::Stamp(), false, true);
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
