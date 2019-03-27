//==========================================================================
// ObTools::JSON: test-parser.cc
//
// Test harness for JSON parser
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-json.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::JSON;

TEST(Parser, TestEmptyStringGivesNULL)
{
  string s;
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::NULL_, value.type);
}

TEST(Parser, TestIntegerGivesInteger)
{
  string s("1234567890123456789");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::INTEGER, value.type);
  ASSERT_EQ(1234567890123456789LL, value.n);
}

TEST(Parser, TestNegativeIntegerGivesInteger)
{
  string s("-1");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::INTEGER, value.type);
  ASSERT_EQ(-1, value.n);
}

TEST(Parser, TestNumberGivesNumber)
{
  string s("3.14");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::NUMBER, value.type);
  ASSERT_EQ(3.14, value.f);
}

TEST(Parser, TestStringGivesString)
{
  string s("\"Hello, world!\"");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::STRING, value.type);
  ASSERT_EQ("Hello, world!", value.s);
}

TEST(Parser, TestnullGivesNULL)
{
  string s("null");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::NULL_, value.type);
}

TEST(Parser, TesttrueGivesTRUE)
{
  string s("true");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::TRUE, value.type);
}

TEST(Parser, TestfalseGivesFALSE)
{
  string s("false");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::FALSE, value.type);
}

TEST(Parser, TestRandomBarewordFails)
{
  string s("foo");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

TEST(Parser, TestEmptyObjectIsEmpty)
{
  string s("{}");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::OBJECT, value.type);
  ASSERT_EQ(0, value.o.size());
}

TEST(Parser, TestObjectWithSingleProperty)
{
  string s("{ foo: 1 }");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::OBJECT, value.type);
  ASSERT_EQ(1, value.o.size());
  Value v1 = value.o["foo"];
  ASSERT_EQ(Value::INTEGER, v1.type);
  ASSERT_EQ(1, v1.n);
}

TEST(Parser, TestObjectWithTwoProperties)
{
  string s("{ foo: \"FOO\", bar: true }");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::OBJECT, value.type);
  ASSERT_EQ(2, value.o.size());
  Value v1 = value.o["foo"];
  ASSERT_EQ(Value::STRING, v1.type);
  ASSERT_EQ("FOO", v1.s);
  Value v2 = value.o["bar"];
  ASSERT_EQ(Value::TRUE, v2.type);
}

TEST(Parser, TestUnclosedObjectFails)
{
  string s("{");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

TEST(Parser, TestObjectMissingColonFails)
{
  string s("{ foo bar }");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

TEST(Parser, TestObjectMissingValueFails)
{
  string s("{ foo: }");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

TEST(Parser, TestObjectMissingCommaFails)
{
  string s("{ foo:1 bar:2 }");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

TEST(Parser, TestEmptyArrayIsEmpty)
{
  string s("[]");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::ARRAY, value.type);
  ASSERT_EQ(0, value.a.size());
}

TEST(Parser, TestArrayWithSingleElement)
{
  string s("[ 1 ]");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::ARRAY, value.type);
  ASSERT_EQ(1, value.a.size());
  Value v1 = value.a[0];
  ASSERT_EQ(Value::INTEGER, v1.type);
  ASSERT_EQ(1, v1.n);
}

TEST(Parser, TestArrayWithTwoElements)
{
  string s("[ \"foo\", true ]");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_NO_THROW(value = parser.read_value());
  ASSERT_EQ(Value::ARRAY, value.type);
  ASSERT_EQ(2, value.a.size());
  Value v1 = value.a[0];
  ASSERT_EQ(Value::STRING, v1.type);
  ASSERT_EQ("foo", v1.s);
  Value v2 = value.a[1];
  ASSERT_EQ(Value::TRUE, v2.type);
}

TEST(Parser, TestArrayMissingCommaFails)
{
  string s("[ 1 2 ]");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

TEST(Parser, TestUnclosedArrayFails)
{
  string s("[");
  istringstream input(s);
  Parser parser(input);
  Value value;
  ASSERT_THROW(value = parser.read_value(), JSON::Exception);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
