//==========================================================================
// ObTools::Text: test-csv.cc
//
// GTest harness for text library CSV functions
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(CSVTest, TestReadEmptyLine)
{
  const string line("");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_TRUE(vars.empty());
}

TEST(CSVTest, TestReadSingleVar)
{
  const string line("foo");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(1, vars.size());
  EXPECT_EQ("foo", vars[0]);
}

TEST(CSVTest, TestReadQuotedVar)
{
  const string line("\"foo\"");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(1, vars.size());
  EXPECT_EQ("foo", vars[0]);
}

TEST(CSVTest, TestEmptyQuotedVar)
{
  const string line("\"\"");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(1, vars.size());
  EXPECT_EQ("", vars[0]);
}

TEST(CSVTest, TestReadCommaSeparatedVars)
{
  const string line("foo,bar,splat");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ("foo", vars[0]);
  EXPECT_EQ("bar", vars[1]);
  EXPECT_EQ("splat", vars[2]);
}

TEST(CSVTest, TestReadCommaSeparatedVarsWithEmptyFields)
{
  const string line(",bar,");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ("", vars[0]);
  EXPECT_EQ("bar", vars[1]);
  EXPECT_EQ("", vars[2]);
}

TEST(CSVTest, TestReadCommaSeparatedVarsWithLeadingAndTrailingSpaces)
{
  const string line(" foo, bar ,splat ");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ(" foo", vars[0]);
  EXPECT_EQ(" bar ", vars[1]);
  EXPECT_EQ("splat ", vars[2]);
}

TEST(CSVTest, TestReadTabSeparatedVars)
{
  const string line("foo\tbar\tsplat");
  Text::CSV csv('\t');
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ("foo", vars[0]);
  EXPECT_EQ("bar", vars[1]);
  EXPECT_EQ("splat", vars[2]);
}

TEST(CSVTest, TestReadCommaSeparatedVarsWithQuotedComma)
{
  const string line("foo,\"bar,bar\",splat");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ("foo", vars[0]);
  EXPECT_EQ("bar,bar", vars[1]);
  EXPECT_EQ("splat", vars[2]);
}

TEST(CSVTest, TestReadCommaSeparatedVarsWithQuotedQuote)
{
  // Note both \" and "" escaping
  const string line("foo,\"bar\"\"bar\",splat");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ("foo", vars[0]);
  EXPECT_EQ("bar\"bar", vars[1]);
  EXPECT_EQ("splat", vars[2]);
}

TEST(CSVTest, TestReadCommaSeparatedVarsWithQuotedVarWithSpacesAround)
{
  // Note both \" and "" escaping
  const string line("foo, \"bar bar\" ,splat");
  Text::CSV csv;
  vector<string> vars;
  csv.read_line(line, vars);
  ASSERT_EQ(3, vars.size());
  EXPECT_EQ("foo", vars[0]);
  EXPECT_EQ(" bar bar ", vars[1]);
  EXPECT_EQ("splat", vars[2]);
}

TEST(CSVTest, TestReadMultiCommaSeparatedVars)
{
  // Note both CR-LF, LF and no final line end
  const string text("foo,bar,splat\r\n,,\n\"wibble, wobble\"");
  Text::CSV csv;
  vector<vector<string> > data;
  csv.read(text, data);
  ASSERT_EQ(3, data.size());
  ASSERT_EQ(3, data[0].size());
  EXPECT_EQ("foo", data[0][0]);
  EXPECT_EQ("bar", data[0][1]);
  EXPECT_EQ("splat", data[0][2]);
  ASSERT_EQ(3, data[1].size());
  EXPECT_EQ("", data[1][0]);
  EXPECT_EQ("", data[1][1]);
  EXPECT_EQ("", data[1][2]);
  ASSERT_EQ(1, data[2].size());
  EXPECT_EQ("wibble, wobble", data[2][0]);
}

TEST(CSVTest, TestReadMultiCommaSeparatedVarsWithHeader)
{
  const string text("c1,c2,c3\r\nfoo,bar,splat");
  Text::CSV csv;
  vector<vector<string> > data;
  csv.read(text, data, true);
  ASSERT_EQ(1, data.size());
  ASSERT_EQ(3, data[0].size());
  EXPECT_EQ("foo", data[0][0]);
  EXPECT_EQ("bar", data[0][1]);
  EXPECT_EQ("splat", data[0][2]);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
