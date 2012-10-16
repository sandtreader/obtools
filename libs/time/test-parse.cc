//==========================================================================
// ObTools::Time: test-parse.cc
//
// Test parsing of time stamps from text
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-time.h"
#include <iostream>
#include <limits.h>

using namespace std;
using namespace ObTools;

TEST(ParseTests, TestParseISO8601)
{
  const string iso8601 = "2011-11-23T10:34:52Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ(iso8601, stamp.iso());
}

TEST(ParseTests, TestParseISO8601HighPrecision)
{
  const string iso8601 = "2011-11-23T10:34:52.7429Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ(iso8601, stamp.iso());
}

TEST(ParseTests, TestParseISO8601Space)
{
  const string iso8601 = "2011-11-23 10:34:52Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ("2011-11-23T10:34:52Z", stamp.iso());
}

TEST(ParseTests, TestParseISO8601NoTZ)
{
  const string iso8601 = "2011-11-23T10:34:52";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ("2011-11-23T10:34:52Z", stamp.iso());
}

TEST(ParseTests, TestParseISO8601IncompletePart)
{
  const string iso8601 = "2011-11-23T10:34:5";
  Time::Stamp stamp(iso8601);
  ASSERT_TRUE(!stamp);
}

TEST(ParseTests, TestParseISO8601NoFiller)
{
  const string iso8601 = "20111123T103452Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ("2011-11-23T10:34:52Z", stamp.iso());
}

TEST(ParseTests, TestParseISO8601Lenient)
{
  const string iso8601 = "2011-11-23T";
  Time::Stamp stamp(iso8601, true);
  ASSERT_EQ("2011-11-23T00:00:00Z", stamp.iso());
}

TEST(ParseTests, TestParseISO8601LenientWithHourOnly)
{
  const string iso8601 = "2011-11-23T13";
  Time::Stamp stamp(iso8601, true);
  ASSERT_EQ("2011-11-23T13:00:00Z", stamp.iso());
}

TEST(ParseTests, TestParseISO8601LenientGarbageTime)
{
  const string iso8601 = "2011-11-23Tabcd";
  Time::Stamp stamp(iso8601, true);
  ASSERT_TRUE(!stamp);
}

TEST(ParseTests, TestParseISO8601LenientCutShort)
{
  const string iso8601 = "2011-11-23T103";
  Time::Stamp stamp(iso8601, true);
  ASSERT_TRUE(!stamp);
}

TEST(ParseTests, TestParseISO8601LenientBadSeconds)
{
  const string iso8601 = "2011-11-23T103452.3.2";
  Time::Stamp stamp(iso8601, true);
  ASSERT_TRUE(!stamp);
}

TEST(ParseTests, TestParseRFC822)
{
  const string rfc822 = "Sun, 06 Nov 1994 08:49:37 GMT";
  Time::Stamp stamp(rfc822);
  ASSERT_EQ("1994-11-06T08:49:37Z", stamp.iso());
}

TEST(ParseTests, TestParseRFC822MissingGMT)
{
  const string rfc822 = "Sun, 06 Nov 1994 08:49:37";
  Time::Stamp stamp(rfc822);
  ASSERT_TRUE(!stamp);
}

TEST(ParseTests, TestParseRFC850)
{
  const string rfc850 = "Sunday, 06-Nov-94 08:49:37 GMT";
  Time::Stamp stamp(rfc850);
  ASSERT_EQ("1994-11-06T08:49:37Z", stamp.iso());
}

TEST(ParseTests, TestParseRFC850MissingGMT)
{
  const string rfc850 = "Sunday, 06-Nov-94 08:49:37";
  Time::Stamp stamp(rfc850);
  ASSERT_TRUE(!stamp);
}

TEST(ParseTests, TestParseRFC850Y2K1900)
{
  const string rfc850 = "Sunday, 07-Feb-37 06:28:15 GMT";
  Time::Stamp stamp(rfc850);
  ASSERT_EQ("1937-02-07T06:28:15Z", stamp.iso());
}

TEST(ParseTests, TestParseRFC850Y2K2000)
{
  // Max NTP timestamp
  const string rfc850 = "Sunday, 07-Feb-36 06:28:15 GMT";
  Time::Stamp stamp(rfc850);
  ASSERT_EQ("2036-02-07T06:28:15Z", stamp.iso());
}

TEST(ParseTests, TestParseAsctime)
{
  const string asc = "Sun Nov  6 08:49:37 1994";
  Time::Stamp stamp(asc);
  ASSERT_EQ("1994-11-06T08:49:37Z", stamp.iso());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
