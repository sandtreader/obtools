//==========================================================================
// ObTools::Time: test-text-parse.cc
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

TEST(StampTests, TestParseISO8601)
{
  const string iso8601 = "2011-11-23T10:34:52Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ(iso8601, stamp.iso());
}

TEST(StampTests, TestParseISO8601HighPrecision)
{
  const string iso8601 = "2011-11-23T10:34:52.7429Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ(iso8601, stamp.iso());
}

TEST(StampTests, TestParseISO8601Space)
{
  const string iso8601 = "2011-11-23 10:34:52Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ("2011-11-23T10:34:52Z", stamp.iso());
}

TEST(StampTests, TestParseISO8601NoTZ)
{
  const string iso8601 = "2011-11-23T10:34:52";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ("2011-11-23T10:34:52Z", stamp.iso());
}

TEST(StampTests, TestParseISO8601IncompletePart)
{
  const string iso8601 = "2011-11-23T10:34:5";
  Time::Stamp stamp(iso8601);
  ASSERT_TRUE(!stamp);
}

TEST(StampTests, TestParseISO8601NoFiller)
{
  const string iso8601 = "20111123T103452Z";
  Time::Stamp stamp(iso8601);
  ASSERT_EQ("2011-11-23T10:34:52Z", stamp.iso());
}

TEST(StampTests, TestParseISO8601Lenient)
{
  const string iso8601 = "2011-11-23T";
  Time::Stamp stamp(iso8601, true);
  ASSERT_EQ("2011-11-23T00:00:00Z", stamp.iso());
}

TEST(StampTests, TestParseISO8601LenientWithHour)
{
  const string iso8601 = "2011-11-23T13";
  Time::Stamp stamp(iso8601, true);
  ASSERT_EQ("2011-11-23T13:00:00Z", stamp.iso());
}

TEST(StampTests, TestParseISO8601LenientGarbageTime)
{
  const string iso8601 = "2011-11-23Tabcd";
  Time::Stamp stamp(iso8601, true);
  ASSERT_TRUE(!stamp);
}

TEST(StampTests, TestParseISO8601LenientCutShort)
{
  const string iso8601 = "2011-11-23T103";
  Time::Stamp stamp(iso8601, true);
  ASSERT_EQ("2011-11-23T10:00:00Z", stamp.iso());
}

TEST(StampTests, TestParseISO8601LenientBadSeconds)
{
  const string iso8601 = "2011-11-23T103452.3.2";
  Time::Stamp stamp(iso8601, true);
  ASSERT_TRUE(!stamp);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
