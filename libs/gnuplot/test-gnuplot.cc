//==========================================================================
// ObTools::Gnuplot: test-gnuplot.cc
//
// Test harness for gnuplot output library
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gnuplot.h"
#include <sstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(GnuplotTest, TestOutputHeader)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "Test Label");
  }
  string result = oss.str();
  EXPECT_NE(string::npos, result.find("set terminal png"));
  EXPECT_NE(string::npos,
    result.find("plot '-' using 1:2 title \"Test Label\" with lines"));
  EXPECT_NE(string::npos, result.find("EOF"));
}

TEST(GnuplotTest, TestAddPoint)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "data");
    output.add_point(1.0, 2.0);
    output.add_point(3.0, 4.0);
  }
  string result = oss.str();
  EXPECT_NE(string::npos, result.find("\t1 2"));
  EXPECT_NE(string::npos, result.find("\t3 4"));
}

TEST(GnuplotTest, TestPrecision)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "precise");
    output.add_point(1.12345678901234567, 2.98765432109876543);
  }
  string result = oss.str();
  // Verify high precision (17 significant digits)
  EXPECT_NE(string::npos, result.find("1.1234567890123457"));
}

TEST(GnuplotTest, TestNegativeValues)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "neg");
    output.add_point(-1.5, -2.5);
  }
  string result = oss.str();
  EXPECT_NE(string::npos, result.find("\t-1.5 -2.5"));
}

TEST(GnuplotTest, TestZeroValues)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "zero");
    output.add_point(0.0, 0.0);
  }
  string result = oss.str();
  EXPECT_NE(string::npos, result.find("\t0 0"));
}

TEST(GnuplotTest, TestDestructorWritesEOF)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "eof-test");
    output.add_point(1.0, 2.0);
  }
  string result = oss.str();
  // EOF should be the last meaningful line
  auto eof_pos = result.rfind("EOF");
  ASSERT_NE(string::npos, eof_pos);
  // Only whitespace/newline should follow EOF
  auto after_eof = result.substr(eof_pos + 3);
  EXPECT_TRUE(after_eof.find_first_not_of("\n\r ") == string::npos);
}

TEST(GnuplotTest, TestEmptyPlotNoPoints)
{
  ostringstream oss;
  {
    Gnuplot::Output output(oss, "empty");
  }
  string result = oss.str();
  // Should still have header and EOF
  EXPECT_NE(string::npos, result.find("set terminal png"));
  EXPECT_NE(string::npos, result.find("EOF"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
