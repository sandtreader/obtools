//==========================================================================
// ObTools::Channel: test-limited.cc
//
// Test harness for Limited Reader
//
// Copyright (c) 2014 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test no error thrown when reading to limit
TEST(LimitedReaderTest, TestOKWhenReadingToLimit)
{
  int limit(21);
  string data("There was an old lady of 92, parlez vous");
  Channel::StringReader sr(data);
  Channel::LimitedReader lr(sr, limit);

  vector<char> out(limit);
  ASSERT_NO_THROW(lr.read(&out[0], limit));
}

//--------------------------------------------------------------------------
// Test error thrown when attempting to read beyond limit
TEST(LimitedReaderTest, TestErrorsWhenReadingBeyondLimit)
{
  int limit(10);
  string data("Did a fart and away it blew, parlez vous");
  Channel::StringReader sr(data);
  Channel::LimitedReader lr(sr, limit);

  vector<char> out(limit + 1);
  ASSERT_THROW(lr.read(&out[0], limit + 1), Channel::Error);
}

//--------------------------------------------------------------------------
// Test that correct data is read
TEST(LimitedReaderTest, TestReadWorks)
{
  int limit(21);
  string data("The fart went rolling down the street, parlez vous");
  Channel::StringReader sr(data);
  Channel::LimitedReader lr(sr, limit);

  string out;
  lr.read(out, limit);

  ASSERT_EQ(data.substr(0, limit), out);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
