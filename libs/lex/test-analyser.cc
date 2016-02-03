//==========================================================================
// ObTools::Lex: test-analyser.cc
//
// Test harness for lexical analyser
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-lex.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Lex;

TEST(Analyser, TestEmptyStringGivesEND)
{
  string s;
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestWhitespaceOnlyGivesEND)
{
  string s(" \n\t\n");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestNameGivesNameThenEND)
{
  string s("fred");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("fred", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestWhitespaceAndNameGivesName)
{
  string s(" \n\tfred");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("fred", token.value);
}

TEST(Analyser, TestComplexName)
{
  string s("Fred_123");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("Fred_123", token.value);
}

TEST(Analyser, TestIntegerGivesNumberThenEND)
{
  string s("1234");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1234", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestFloatGivesNumberThenEND)
{
  string s("1234.56");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1234.56", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestFloatExponentGivesNumberThenEND)
{
  string s("1234.56E02");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1234.56E02", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestFloatExponentLowerCaseGivesNumberThenEND)
{
  string s("1234.56e02");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1234.56e02", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestFloatExponentWithPlusGivesNumberThenEND)
{
  string s("1234.56E+02");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1234.56E+02", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestFloatExponentWithMinusGivesNumberThenEND)
{
  string s("1234.56E-02");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1234.56E-02", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestEndInExponentFails)
{
  string s("1234.56E");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
}

TEST(Analyser, TestEndInExponentWithMinusFails)
{
  string s("1234.56E-");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
}

TEST(Analyser, TestEndInExponentWithPlusFails)
{
  string s("1234.56E+");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
}

TEST(Analyser, TestNumberAndNameConcatenatedAreSplit)
{
  string s("123Fred");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("123", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("Fred", token.value);
}

TEST(Analyser, TestQuotedStringGivesStringThenEND)
{
  string s("\"hello\"");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::STRING, token.type);
  ASSERT_EQ("hello", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestQuotedStringWithEscapes)
{
  string s("\"\\\\\\\"\\b\\f\\n\\r\\t\\u00Ff\\uabCD\"");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::STRING, token.type);
  ASSERT_EQ("\\\"\b\f\n\r\t\u00ff\uabcd", token.value);
}

TEST(Analyser, TestUnclosedStringFails)
{
  string s("\"hello");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
}

TEST(Analyser, TestUnclosedStringWithEscapeFails)
{
  string s("\"hello\\");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
}

TEST(Analyser, TestUnclosedStringWithUnicodeEscapeFails)
{
  string s("\"hello\\uab");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
}

TEST(Analyser, TestSimpleSymbolGivesSymbolThenEND)
{
  string s("+");
  istringstream input(s);
  Analyser analyser(input);
  analyser.add_symbol("+");
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("+", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestTwoSimpleSymbolsGivesTwoSymbolsThenEND)
{
  string s("++");
  istringstream input(s);
  Analyser analyser(input);
  analyser.add_symbol("+");
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("+", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("+", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestComplexSymbolGivesSymbolThenEND)
{
  string s("++");
  istringstream input(s);
  Analyser analyser(input);
  analyser.add_symbol("++");
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("++", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestComplexSymbolsAreGreedy)
{
  string s("+++");
  istringstream input(s);
  Analyser analyser(input);
  analyser.add_symbol("++");
  analyser.add_symbol("+");
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("++", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("+", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestComplexMixtureOfSymbols)
{
  string s(" fred1++ <= jimX_99---1.0  ");
  istringstream input(s);
  Analyser analyser(input);
  analyser.add_symbol("++");
  analyser.add_symbol("+");
  analyser.add_symbol("--");
  analyser.add_symbol("-");
  analyser.add_symbol("<=");
  analyser.add_symbol("<");
  Token token;
  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("fred1", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("++", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("<=", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NAME, token.type);
  ASSERT_EQ("jimX_99", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("--", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::SYMBOL, token.type);
  ASSERT_EQ("-", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::NUMBER, token.type);
  ASSERT_EQ("1.0", token.value);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}

TEST(Analyser, TestRandomCrapFailsCleanly)
{
  string s("@$#");
  istringstream input(s);
  Analyser analyser(input);
  Token token;
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);
  ASSERT_THROW(token = analyser.read_token(), Lex::Exception);

  ASSERT_NO_THROW(token = analyser.read_token());
  ASSERT_EQ(Token::END, token.type);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
