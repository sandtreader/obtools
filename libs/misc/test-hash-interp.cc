//==========================================================================
// ObTools::Misc: test-hash-interp.cc
//
// Test harness for hash interpolator functions
//
// Copyright (c) 2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

TEST(HashInterpolatorTests, TestNOOPDoesntChangePropertyList)
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "two");
  string orig = pl.str();

  Misc::HashInterpolator hi;
  // No hashes
  hi.augment(pl);

  ASSERT_EQ(orig, pl.str());
}

TEST(HashInterpolatorTests, TestSingleHashGeneratesAValue)
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "two");

  Misc::HashInterpolator hi;
  hi.add_hash("hash", 10, "$foo$bar");
  hi.augment(pl);

  ASSERT_TRUE(pl.has("hash"));
}

TEST(HashInterpolatorTests, TestHashesOnDifferentInputsDiffer)
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "two");

  Misc::HashInterpolator hi;
  hi.add_hash("hash", 10, "$foo$bar");

  // First try
  hi.augment(pl);
  int n1 = pl.get_int("hash");

  // Change input and try again
  pl.add("bar", "three");
  hi.augment(pl);
  int n2 = pl.get_int("hash");

  ASSERT_NE(n1, n2) << "Hashes aren't different!";
}

TEST(HashInterpolatorTests, TestBulkHashesAssortRandomly)
{
  int spread = 10;
  int tries = 1000000;

  Misc::HashInterpolator hi;
  hi.add_hash("hash", spread, "$i");
  map<int, int> counts;

  // Hash successive values and count the number in each bucket
  for(int i=0; i<tries; i++)
  {
    Misc::PropertyList pl;
    pl.add("i", i);
    hi.augment(pl);

    int n = pl.get_int("hash");
    counts[n]++;
  }

  int highest = 0;
  int lowest = tries;
  int sum = 0;
  for(map<int, int>::iterator p=counts.begin(); p!=counts.end(); ++p)
  {
    // All values must lie in spread range
    ASSERT_LE(0, p->first) << "Hash out of range";
    ASSERT_GT(spread, p->first) << "Hash out of range";

    if (p->second > highest) highest = p->second;
    if (p->second < lowest) lowest = p->second;
    sum += p->second;
  }

  ASSERT_EQ(tries, sum);

  int expected = tries / spread;

  // Allow 1% error either way
  EXPECT_LE(expected*99/100, lowest);
  EXPECT_GE(expected*101/100, highest);
}

TEST(HashInterpolatorTests, TestReadingHashesFromXML)
{
  string xml =
    "<hashes>\n"
    "  <hash name='h1' modulus='10'>$foo$bar</hash>\n"
    "  <hash name='h2' modulus='1000'>$wombats are go!</hash>\n"
    "</hashes>\n";

  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& root=parser.get_root();
  Misc::HashInterpolator hi1(root);

  Misc::HashInterpolator hi2;
  hi2.add_hash("h1", 10, "$foo$bar");
  hi2.add_hash("h2", 1000, "$wombats are go!");

  ASSERT_EQ(hi1, hi2);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



