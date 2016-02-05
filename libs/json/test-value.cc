//==========================================================================
// ObTools::JSON: test-value.cc
//
// Test harness for JSON value writing etc.
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-json.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::JSON;

TEST(Value, TestObjectSetter)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", "hello");
  ASSERT_EQ(2, value.o.size());
  Value& v1 = value.o["foo"];
  EXPECT_EQ(Value::INTEGER, v1.type);
  EXPECT_EQ(1, v1.n);
  Value& v2 = value.o["bar"];
  EXPECT_EQ(Value::STRING, v2.type);
  EXPECT_EQ("hello", v2.s);
}

TEST(Value, TestArrayAdder)
{
  Value value(Value::ARRAY);
  value.add(1);
  value.add("hello");
  ASSERT_EQ(2, value.a.size());
  Value& v1 = value.a[0];
  EXPECT_EQ(Value::INTEGER, v1.type);
  EXPECT_EQ(1, v1.n);
  Value& v2 = value.a[1];
  EXPECT_EQ(Value::STRING, v2.type);
  EXPECT_EQ("hello", v2.s);
}

TEST(Value, TestWritingNull)
{
  Value value;
  ostringstream out;
  out << value;
  ASSERT_EQ("null", out.str());
}

TEST(Value, TestWritingTrue)
{
  Value value(Value::TRUE);
  ostringstream out;
  out << value;
  ASSERT_EQ("true", out.str());
}

TEST(Value, TestWritingFalse)
{
  Value value(Value::FALSE);
  ostringstream out;
  out << value;
  ASSERT_EQ("false", out.str());
}

TEST(Value, TestWritingNumber)
{
  Value value(Value::NUMBER, 3.1415);
  ostringstream out;
  out << value;
  ASSERT_EQ("3.1415", out.str());
}

TEST(Value, TestWritingBigIntegerStaysIntegral)
{
  Value value(12345678901234567890ULL);
  ostringstream out;
  out << value;
  ASSERT_EQ("12345678901234567890", out.str());
}

TEST(Value, TestWritingString)
{
  Value value("foo");
  ostringstream out;
  out << value;
  ASSERT_EQ("\"foo\"", out.str());
}

TEST(Value, TestWritingStringEncoding)
{
  Value value("\\\"\b\f\n\r\t\uabcd");
  ostringstream out;
  out << value;
  ASSERT_EQ("\"\\\\\\\"\\b\\f\\n\\r\\t\\uabcd\"", out.str());
}

TEST(Value, TestWritingEmptyObject)
{
  Value value(Value::OBJECT);
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("{}", out.str());
}

TEST(Value, TestWritingSinglePropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("{\"foo\":1}", out.str());
}

TEST(Value, TestWritingTwoPropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", 2);
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("{\"bar\":2,\"foo\":1}", out.str());
}

TEST(Value, TestWritingNestedObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", Value(Value::OBJECT));
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("{\"bar\":{},\"foo\":1}", out.str());
}

TEST(Value, TestWritingEmptyArray)
{
  Value value(Value::ARRAY);
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("[]", out.str());
}

TEST(Value, TestWritingSingleElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("[1]", out.str());
}

TEST(Value, TestWritingTwoElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  value.add(2);
  ostringstream out;
  value.write_to(out);  // Not pretty
  ASSERT_EQ("[1,2]", out.str());
}

TEST(Value, TestWritingPrettyEmptyObject)
{
  Value value(Value::OBJECT);
  ostringstream out;
  out << value;  // pretty
  ASSERT_EQ("{}", out.str());
}

TEST(Value, TestWritingPrettySinglePropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  ostringstream out;
  out << value;  // pretty
  cout << out.str() << endl;
  ASSERT_EQ("{\n  \"foo\": 1\n}\n", out.str());
}

TEST(Value, TestWritingPrettyTwoPropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", 2);
  ostringstream out;
  out << value;  // pretty
  cout << out.str() << endl;
  ASSERT_EQ("{\n  \"bar\": 2,\n  \"foo\": 1\n}\n", out.str());
}

TEST(Value, TestWritingPrettyNestedEmptyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", Value(Value::OBJECT));
  ostringstream out;
  out << value;  // pretty
  cout << out.str() << endl;
  ASSERT_EQ("{\n  \"bar\": {},\n  \"foo\": 1\n}\n", out.str());
}

TEST(Value, TestWritingPrettyNestedNonEmptyObject)
{
  Value inner(Value::OBJECT);
  inner.set("splat", 3);
  inner.set("wombat", 4);

  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", inner);
  ostringstream out;
  out << value;  // pretty
  cout << out.str() << endl;
  ASSERT_EQ("{\n  \"bar\":\n  {\n    \"splat\": 3,\n    \"wombat\": 4\n  },\n  \"foo\": 1\n}\n", out.str());
}

TEST(Value, TestWritingPrettyEmptyArray)
{
  Value value(Value::ARRAY);
  ostringstream out;
  out << value;  // pretty
  ASSERT_EQ("[]", out.str());
}

TEST(Value, TestWritingPrettySingleElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  ostringstream out;
  out << value;  // pretty
  ASSERT_EQ("[ 1 ]", out.str());
}

TEST(Value, TestWritingPrettyTwoElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  value.add(2);
  ostringstream out;
  out << value;  // pretty
  ASSERT_EQ("[ 1, 2 ]", out.str());
}

TEST(Value, TestWritingPrettyNestedArray)
{
  Value inner2(Value::OBJECT);
  inner2.set("foo", 3);

  Value inner(Value::ARRAY);
  inner.add(2);
  inner.add(inner2);

  Value value(Value::ARRAY);
  value.add(inner);
  value.add(1);
  ostringstream out;
  out << value;  // pretty
  cout << out.str() << endl;
  ASSERT_EQ("[\n  [\n    2,\n    {\n      \"foo\": 3\n    }\n  ],\n  1\n]\n", out.str());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
