//==========================================================================
// ObTools::Lex: test-edge-cases.cc
//
// Test harness for lexer edge cases - error paths
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-lex.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ObTools;

TEST(LexEdgeTest, TestBadNumberExponentThrows)
{
  // A number like "1e!" should trigger the bad exponent error
  istringstream iss("1e!");
  Lex::Analyser lex(iss);
  EXPECT_THROW(lex.read_token(), Lex::Exception);
}

TEST(LexEdgeTest, TestBadStringEscapeThrows)
{
  // A string with an unrecognised escape like "\q" should throw
  istringstream iss("\"\\q\"");
  Lex::Analyser lex(iss);
  EXPECT_THROW(lex.read_token(), Lex::Exception);
}

TEST(LexEdgeTest, TestValidNumberExponent)
{
  istringstream iss("1e5");
  Lex::Analyser lex(iss);
  Lex::Token tok = lex.read_token();
  EXPECT_EQ(Lex::Token::NUMBER, tok.type);
  EXPECT_EQ("1e5", tok.value);
}

TEST(LexEdgeTest, TestValidStringEscapes)
{
  istringstream iss("\"\\n\\t\\r\\\\\\\"\"");
  Lex::Analyser lex(iss);
  Lex::Token tok = lex.read_token();
  EXPECT_EQ(Lex::Token::STRING, tok.type);
}

TEST(LexEdgeTest, TestDotNotFollowedByDigitIsSymbol)
{
  // A '.' not followed by a digit triggers symbol path, but '.' must be
  // registered via add_symbol() first - without it, an exception is thrown
  istringstream iss(".abc");
  Lex::Analyser lex(iss);
  EXPECT_THROW(lex.read_token(), Lex::Exception);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
