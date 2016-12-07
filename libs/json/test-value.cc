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

TEST(Value, TestValidity)
{
  Value nv;
  ASSERT_TRUE(!nv);
  Value v(Value::NULL_);
  ASSERT_FALSE(!v);
}

TEST(Value, TestIsTrue)
{
  Value nv;
  ASSERT_FALSE(nv.is_true());
  Value tv(Value::TRUE);
  ASSERT_TRUE(tv.is_true());
  Value fv(Value::FALSE);
  ASSERT_FALSE(fv.is_true());
  Value v1(1);
  ASSERT_TRUE(v1.is_true());
  Value v0(0);
  ASSERT_FALSE(v0.is_true());
}

TEST(Value, TestAsStr)
{
  Value v("foo");
  ASSERT_EQ("foo", v.as_str());

  Value nv;
  ASSERT_EQ("bar", nv.as_str("bar"));
}

TEST(Value, TestAsInt)
{
  Value v(42);
  ASSERT_EQ(42, v.as_int());

  Value nv;
  ASSERT_EQ(99, nv.as_int(99));
}

TEST(Value, TestAsFloat)
{
  Value vf(Value::NUMBER, 42.9);
  ASSERT_EQ(42.9, vf.as_float());

  Value vi(42);
  ASSERT_EQ(42.0, vi.as_float());

  Value nv;
  ASSERT_EQ(99.9, nv.as_float(99.9));
}

TEST(Value, TestObjectSetter)
{
  Value value(Value::OBJECT);
  value.set("foo", 0);
  value.set("bar", "hello");
  ASSERT_EQ(2, value.o.size());
  Value& v1 = value.o["foo"];
  EXPECT_EQ(Value::INTEGER, v1.type);
  EXPECT_EQ(0, v1.n);
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

TEST(Value, TestObjectGetter)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  const Value& v = value["foo"];
  EXPECT_EQ(Value::INTEGER, v.type);
  EXPECT_EQ(1, v.n);

  const Value& nv = value["bar"];
  EXPECT_TRUE(!nv);
}

TEST(Value, TestArrayGetter)
{
  Value value(Value::ARRAY);
  value.add(1);
  const Value& v = value[0];
  EXPECT_EQ(Value::INTEGER, v.type);
  EXPECT_EQ(1, v.n);

  const Value& nv1 = value[-1];
  EXPECT_TRUE(!nv1);
  const Value& nv2 = value[1];
  EXPECT_TRUE(!nv2);
}

TEST(Value, TestArraySize)
{
  Value value(Value::ARRAY);
  value.add(1);
  value.add(2);
  ASSERT_EQ(2, value.size());

  Value nv;
  ASSERT_EQ(0, nv.size());
}

TEST(Value, TestWritingUnsetThrows)
{
  Value value;
  ostringstream out;
  ASSERT_THROW(out << value, JSON::Exception);
}

TEST(Value, TestWritingNull)
{
  Value value(Value::NULL_);
  ASSERT_EQ("null", value.str());
}

TEST(Value, TestWritingTrue)
{
  Value value(Value::TRUE);
  ASSERT_EQ("true", value.str());
}

TEST(Value, TestWritingFalse)
{
  Value value(Value::FALSE);
  ASSERT_EQ("false", value.str());
}

TEST(Value, TestWritingNumber)
{
  Value value(Value::NUMBER, 3.1415);
  ASSERT_EQ("3.1415", value.str());
}

TEST(Value, TestWritingBigIntegerStaysIntegral)
{
  Value value(12345678901234567890ULL);
  ASSERT_EQ("12345678901234567890", value.str());
}

TEST(Value, TestWritingString)
{
  Value value("foo");
  ASSERT_EQ("\"foo\"", value.str());
}

TEST(Value, TestWritingStringEncoding)
{
  Value value("\\\"\b\f\n\r\t\uabcd");
  ASSERT_EQ("\"\\\\\\\"\\b\\f\\n\\r\\t\\uabcd\"", value.str());
}

TEST(Value, TestWritingEmptyObject)
{
  Value value(Value::OBJECT);
  ASSERT_EQ("{}", value.str());
}

TEST(Value, TestWritingSinglePropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  ASSERT_EQ("{\"foo\":1}", value.str());
}

TEST(Value, TestWritingTwoPropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", 2);
  ASSERT_EQ("{\"bar\":2,\"foo\":1}", value.str());
}

TEST(Value, TestWritingNestedObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", Value(Value::OBJECT));
  ASSERT_EQ("{\"bar\":{},\"foo\":1}", value.str());
}

TEST(Value, TestWritingEmptyArray)
{
  Value value(Value::ARRAY);
  ASSERT_EQ("[]", value.str());
}

TEST(Value, TestWritingSingleElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  ASSERT_EQ("[1]", value.str());
}

TEST(Value, TestWritingTwoElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  value.add(2);
  ASSERT_EQ("[1,2]", value.str());
}

TEST(Value, TestWritingPrettyEmptyObject)
{
  Value value(Value::OBJECT);
  ASSERT_EQ("{}", value.str(true)); // pretty
}

TEST(Value, TestWritingPrettySinglePropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  ASSERT_EQ("{\n  \"foo\": 1\n}\n", value.str(true)); // pretty
}

TEST(Value, TestWritingPrettyTwoPropertyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", 2);
  ASSERT_EQ("{\n  \"bar\": 2,\n  \"foo\": 1\n}\n", value.str(true)); // pretty
}

TEST(Value, TestWritingPrettyNestedEmptyObject)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", Value(Value::OBJECT));
  ASSERT_EQ("{\n  \"bar\": {},\n  \"foo\": 1\n}\n", value.str(true)); // pretty
}

TEST(Value, TestWritingPrettyNestedNonEmptyObject)
{
  Value inner(Value::OBJECT);
  inner.set("splat", 3);
  inner.set("wombat", 4);

  Value value(Value::OBJECT);
  value.set("foo", 1);
  value.set("bar", inner);
  ASSERT_EQ("{\n  \"bar\":\n  {\n    \"splat\": 3,\n    \"wombat\": 4\n  },\n  \"foo\": 1\n}\n", value.str(true)); // pretty
}

TEST(Value, TestWritingPrettyEmptyArray)
{
  Value value(Value::ARRAY);
  ASSERT_EQ("[]", value.str(true));  // pretty
}

TEST(Value, TestWritingPrettySingleElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  ASSERT_EQ("[ 1 ]", value.str(true));  // pretty
}

TEST(Value, TestWritingPrettyTwoElementArray)
{
  Value value(Value::ARRAY);
  value.add(1);
  value.add(2);
  ASSERT_EQ("[ 1, 2 ]", value.str(true));  // pretty
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
  ASSERT_EQ("[\n  [\n    2,\n    {\n      \"foo\": 3\n    }\n  ],\n  1\n]\n",
            value.str(true));  // pretty
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
