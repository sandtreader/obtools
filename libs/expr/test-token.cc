//==========================================================================
// ObTools::Expression: test-token.cc
//
// Test harness for expression tokeniser
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-expr.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Expression;

TEST(Tokeniser, TestEmptyStringGivesEOT)
{
  string input;
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestWhitespaceOnlyGivesEOT)
{
  string input(" \n\t\n");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestNameGivesNameThenEOT)
{
  string input("fred");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ(input, token.name);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestWhitespaceAndNameGivesName)
{
  string input(" \n\tfred");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("fred", token.name);
}

TEST(Tokeniser, TestComplexName)
{
  string input("Fred_123");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ(input, token.name);
}

TEST(Tokeniser, TestIntegerGivesNumberThenEOT)
{
  string input("1234");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ(1234.0, token.value);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestFloatGivesNumberThenEOT)
{
  string input("1234.56");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ(1234.56, token.value);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestNumberAndNameConcatenatedAreSplit)
{
  string input("123Fred");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ(123.0, token.value);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("Fred", token.name);
}

TEST(Tokeniser, TestSimpleOperators)
{
  string input("+-*/()");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::PLUS, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::MINUS, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::MUL, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::DIV, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::LPAR, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::RPAR, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestOptionallyDoubledOperators)
{
  string input("&|=&&||===");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::AND, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::OR, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EQ, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::AND, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::OR, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EQ, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EQ, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestComparisonOperators)
{
  string input("<<=>>=<>!=!!");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::LT, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::LTEQ, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::GT, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::GTEQ, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NE, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NE, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NOT, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::NOT, token.type);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

TEST(Tokeniser, TestRandomCrapFailsCleanly)
{
  string input("@$#");
  Tokeniser tokeniser(input);
  Token token;
  ASSERT_THROW(token = tokeniser.read_token(), Expression::Exception);
  ASSERT_THROW(token = tokeniser.read_token(), Expression::Exception);
  ASSERT_THROW(token = tokeniser.read_token(), Expression::Exception);

  ASSERT_NO_THROW(token = tokeniser.read_token());
  ASSERT_EQ(Token::EOT, token.type);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
