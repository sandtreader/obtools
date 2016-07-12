//==========================================================================
// ObTools::JSON: test-real-data.cc
//
// Test harness for JSON using real data samples from:
// http://www.sitepoint.com/10-example-json-files/
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-json.h"
#include <fstream>

using namespace std;
using namespace ObTools;
using namespace ObTools::JSON;

void roundtrip(const char *fn)
{
  ifstream input(fn);
  ASSERT_FALSE(!input);

  Parser parser(input);
  Value value;
  try
  {
    value = parser.read_value();
  }
  catch (Exception e)
  {
    FAIL() << "Raw parse failed: " << e.error;
  }

  // Round trip it
  ostringstream oss;
  oss << value;
  string regen = oss.str();
  cout << regen;

  istringstream iss(regen);
  Parser parser2(iss);
  Value value2;
  try
  {
    value2 = parser2.read_value();
  }
  catch (Exception e)
  {
    FAIL() << "Regen parse failed: " << e.error;
  }

  ostringstream oss2;
  oss2 << value2;
  ASSERT_EQ(regen, oss2.str());
}

TEST(RealData, TestRoundTripOfTwitterData)
{
  roundtrip("tests/twitter.json");
}

TEST(RealData, TestRoundTripOfFacebookData)
{
  roundtrip("tests/facebook.json");
}

TEST(RealData, TestRoundTripOfJSONDotOrgData)
{
  roundtrip("tests/json.org.json");
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
