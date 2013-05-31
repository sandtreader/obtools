//==========================================================================
// ObTools::Expression: test-eval.cc
//
// Test harness for expression evaluator
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-expr.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Expression;

TEST(Evaluator, TestSimpleNumbers)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(1234.0, evaluator.evaluate("1234"));
  ASSERT_EQ(1234.567, evaluator.evaluate("1234.567"));
}

TEST(Evaluator, TestSimpleOperators)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(5.0, evaluator.evaluate("3+2"));
  ASSERT_EQ(1.0, evaluator.evaluate("3-2"));
  ASSERT_EQ(6.0, evaluator.evaluate("3*2"));
  ASSERT_EQ(1.5, evaluator.evaluate("3/2"));

  ASSERT_EQ(1.0, evaluator.evaluate("2=2"));
  ASSERT_EQ(1.0, evaluator.evaluate("2==2"));
  ASSERT_EQ(0.0, evaluator.evaluate("3=2"));
  ASSERT_EQ(0.0, evaluator.evaluate("3==2"));

  ASSERT_EQ(1.0, evaluator.evaluate("2<>3"));
  ASSERT_EQ(1.0, evaluator.evaluate("2!=3"));
  ASSERT_EQ(0.0, evaluator.evaluate("2<>2"));
  ASSERT_EQ(0.0, evaluator.evaluate("2!=2"));

  ASSERT_EQ(1.0, evaluator.evaluate("2<3"));
  ASSERT_EQ(0.0, evaluator.evaluate("3<2"));
  ASSERT_EQ(1.0, evaluator.evaluate("3>2"));
  ASSERT_EQ(0.0, evaluator.evaluate("2>3"));
  ASSERT_EQ(1.0, evaluator.evaluate("2<=3"));
  ASSERT_EQ(1.0, evaluator.evaluate("2<=2"));
  ASSERT_EQ(0.0, evaluator.evaluate("3<=2"));
  ASSERT_EQ(1.0, evaluator.evaluate("3>=2"));
  ASSERT_EQ(1.0, evaluator.evaluate("2>=2"));
  ASSERT_EQ(0.0, evaluator.evaluate("2>=3"));

  ASSERT_EQ(1.0, evaluator.evaluate("1&1"));
  ASSERT_EQ(1.0, evaluator.evaluate("1&&1"));
  ASSERT_EQ(0.0, evaluator.evaluate("1&0"));
  ASSERT_EQ(0.0, evaluator.evaluate("1&&0"));
  ASSERT_EQ(1.0, evaluator.evaluate("0|1"));
  ASSERT_EQ(1.0, evaluator.evaluate("0||1"));
  ASSERT_EQ(0.0, evaluator.evaluate("0|0"));
  ASSERT_EQ(0.0, evaluator.evaluate("0||0"));

  ASSERT_EQ(0.0, evaluator.evaluate("!1"));
  ASSERT_EQ(1.0, evaluator.evaluate("!0"));

  ASSERT_EQ(-2.0, evaluator.evaluate("-2"));
}

TEST(Evaluator, TestChainedOperators)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(6.0, evaluator.evaluate("2+2+2"));
  ASSERT_EQ(2.0, evaluator.evaluate("2+2-2"));
  ASSERT_EQ(8.0, evaluator.evaluate("2*2*2"));
  ASSERT_EQ(2.0, evaluator.evaluate("2*2/2"));
  ASSERT_EQ(1.0, evaluator.evaluate("1&1&1"));
  ASSERT_EQ(1.0, evaluator.evaluate("0|1|0"));
  ASSERT_EQ(1.0, evaluator.evaluate("--1"));
  ASSERT_EQ(1.0, evaluator.evaluate("!!1"));
}

TEST(Evaluator, TestOperatorPrecedence)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(7.0, evaluator.evaluate("3*2+1"));
  ASSERT_EQ(7.0, evaluator.evaluate("1+3*2"));
  ASSERT_EQ(0.0, evaluator.evaluate("2+2<4"));
  ASSERT_EQ(1.0, evaluator.evaluate("2 < 1*3"));
}

TEST(Evaluator, TestParentheses)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(9.0, evaluator.evaluate("3*(2+1)"));
  ASSERT_EQ(45.0, evaluator.evaluate("5*(3*(2+1))"));
}

TEST(Evaluator, TestComplexExpression)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(1.0, evaluator.evaluate("2+2*2 <= 6.0 && 3+2 == 5"));
}

TEST(Evaluator, TestVariablesDontWorkByDefault)
{
  Evaluator evaluator(cerr);
  ASSERT_EQ(0.0, evaluator.evaluate("foo"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
