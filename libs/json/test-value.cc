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

TEST(Value, TestConstructBinary)
{
  vector<unsigned char> b{42, 99};
  Value v(b);
  EXPECT_EQ(Value::BINARY, v.type);
  EXPECT_EQ("\x2a\x63", v.s);
}

TEST(Value, TestIsTrue)
{
  Value nv;
  ASSERT_FALSE(nv.is_true());
  Value tv(Value::TRUE_);
  ASSERT_TRUE(tv.is_true());
  Value fv(Value::FALSE_);
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

  Value vs("42");
  ASSERT_EQ(42, vs.as_int());

  Value nv;
  ASSERT_EQ(99, nv.as_int(99));
}

TEST(Value, TestAsFloat)
{
  Value vf(42.9);
  ASSERT_EQ(42.9, vf.as_float());

  Value vi(42);
  ASSERT_EQ(42.0, vi.as_float());

  Value vs("42.9");
  ASSERT_EQ(42.9, vs.as_float());

  Value nv;
  ASSERT_EQ(99.9, nv.as_float(99.9));
}

TEST(Value, TestAsBinaryWithBinary)
{
  vector<unsigned char> b{42, 99};
  Value v(b);
  auto b2 = v.as_binary();
  ASSERT_EQ(2, b2.size());
  EXPECT_EQ(byte{42}, b2[0]);
  EXPECT_EQ(byte{99}, b2[1]);
}

TEST(Value, TestAsBinaryWithString)
{
  Value v("KmM=");
  auto b2 = v.as_binary();
  ASSERT_EQ(2, b2.size());
  EXPECT_EQ(byte{42}, b2[0]);
  EXPECT_EQ(byte{99}, b2[1]);
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

TEST(Value, TestWritingUnset)
{
  Value value;
  ASSERT_EQ("undefined", value.str());
}

TEST(Value, TestWritingNull)
{
  Value value(Value::NULL_);
  ASSERT_EQ("null", value.str());
}

TEST(Value, TestWritingTrue)
{
  Value value(Value::TRUE_);
  ASSERT_EQ("true", value.str());
}

TEST(Value, TestWritingFalse)
{
  Value value(Value::FALSE_);
  ASSERT_EQ("false", value.str());
}

TEST(Value, TestWritingNumber)
{
  Value value(3.1415);
  ASSERT_EQ("3.1415", value.str());
}

TEST(Value, TestWritingBigIntegerStaysIntegral)
{
  Value value(int64_t{1234567890123456789LL});
  ASSERT_EQ("1234567890123456789", value.str());
}

TEST(Value, TestWritingNegativeIntegerStaysNegative)
{
  Value value(-1);
  ASSERT_EQ("-1", value.str());
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

TEST(Value, TestWritingBinary)
{
  vector<unsigned char> b{42, 99};
  Value value(b);
  ASSERT_EQ("\"KmM=\"", value.str());
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

TEST(Value, TestCompareSimpleValues)
{
  Value f1(Value::FALSE_);
  Value f2(Value::FALSE_);
  Value t1(Value::TRUE_);
  Value t2(Value::TRUE_);
  Value n1(Value::NULL_);
  Value n2(Value::NULL_);
  Value u1;
  Value u2;

  EXPECT_EQ(f1, f2);
  EXPECT_EQ(t1, t2);
  EXPECT_EQ(n1, n2);
  EXPECT_EQ(u1, u2);

  EXPECT_NE(f1, t1);
  EXPECT_NE(t1, n1);
  EXPECT_NE(n1, u2);
  EXPECT_NE(u2, f1);
}

TEST(Value, TestCompareNumbers)
{
  Value n1(42.0);
  Value n2(42.0);
  Value n3(99.0);
  Value u;

  EXPECT_EQ(n1, n2);
  EXPECT_NE(n1, n3);
  EXPECT_NE(n1, u);
}

TEST(Value, TestCompareIntegers)
{
  Value i1(42);
  Value i2(42);
  Value i3(99);
  Value u;

  EXPECT_EQ(i1, i2);
  EXPECT_NE(i1, i3);
  EXPECT_NE(i1, u);
}

TEST(Value, TestCompareStrings)
{
  Value s1("foo");
  Value s2("foo");
  Value s3("bar");
  Value u;

  EXPECT_EQ(s1, s2);
  EXPECT_NE(s1, s3);
  EXPECT_NE(s1, u);
}


TEST(Value, TestCompareBinary)
{
  vector<unsigned char> b1{42, 99};
  Value bv1(b1);
  vector<unsigned char> b2{42, 99};
  Value bv2(b2);
  vector<unsigned char> b3{42};
  Value bv3(b3);
  Value u;

  EXPECT_EQ(bv1, bv2);
  EXPECT_NE(bv1, bv3);
  EXPECT_NE(bv1, u);
}

TEST(Value, TestCompareObjects)
{
  Value u;
  Value o1(Value::OBJECT);
  o1.set("foo", 42);

  Value o2(Value::OBJECT);
  o2.set("foo", 42);

  Value o3(Value::OBJECT);
  o3.set("foo", 99);

  Value o4(Value::OBJECT);
  o4.set("foo", 42);
  o4.set("bar", 42);

  Value o5(Value::OBJECT);

  EXPECT_EQ(o1, o2);
  EXPECT_NE(o1, o3);
  EXPECT_NE(o1, o4);
  EXPECT_NE(o1, o5);
  EXPECT_NE(o1, u);
}

TEST(Value, TestCompareArrays)
{
  Value u;
  Value a1(Value::ARRAY);
  a1.add("foo");

  Value a2(Value::ARRAY);
  a2.add("foo");

  Value a3(Value::ARRAY);
  a3.add("bar");

  Value a4(Value::ARRAY);
  a4.add("foo");
  a4.add("bar");

  Value a5(Value::ARRAY);

  EXPECT_EQ(a1, a2);
  EXPECT_NE(a1, a3);
  EXPECT_NE(a1, a4);
  EXPECT_NE(a1, a5);
  EXPECT_NE(a1, u);
}

TEST(Value, TestConstGetProperty)
{
  Value value(Value::OBJECT);
  value.set("foo", 42);
  const Value& cv = value;

  const Value& v = cv.get("foo");
  EXPECT_EQ(Value::INTEGER, v.type);
  EXPECT_EQ(42, v.n);

  const Value& nv = cv.get("bar");
  EXPECT_TRUE(!nv);

  // Non-object returns none
  const Value ci(42);
  const Value& nv2 = ci.get("x");
  EXPECT_TRUE(!nv2);
}

TEST(Value, TestConstGetIndex)
{
  Value value(Value::ARRAY);
  value.add(42);
  value.add("hello");
  const Value& cv = value;

  const Value& v = cv.get(0u);
  EXPECT_EQ(Value::INTEGER, v.type);
  EXPECT_EQ(42, v.n);

  const Value& nv = cv.get(99u);
  EXPECT_TRUE(!nv);

  // Non-array returns none
  const Value ci(42);
  const Value& nv2 = ci.get(0u);
  EXPECT_TRUE(!nv2);
}

TEST(Value, TestOperatorStream)
{
  Value value(Value::OBJECT);
  value.set("foo", 1);
  ostringstream oss;
  oss << value;
  // operator<< uses pretty print
  EXPECT_EQ("{\n  \"foo\": 1\n}\n", oss.str());
}

TEST(Value, TestAsBinaryOnNonBinaryNonString)
{
  Value v(42);
  auto b = v.as_binary();
  EXPECT_TRUE(b.empty());

  Value nv;
  auto b2 = nv.as_binary();
  EXPECT_TRUE(b2.empty());
}

TEST(Value, TestWritingBreak)
{
  Value v(Value::BREAK);
  EXPECT_EQ("BREAK", v.str());
}

TEST(Value, TestConstructBinaryFromByteVector)
{
  vector<byte> b{byte{0x01}, byte{0x02}};
  Value v(b);
  EXPECT_EQ(Value::BINARY, v.type);
  EXPECT_EQ(2, v.s.size());
}

TEST(Value, TestWritingSupplementaryUnicode)
{
  // U+1F600 (grinning face) encodes as UTF-8: F0 9F 98 80
  // On non-Windows this should produce \\u++++
  string emoji = "\xF0\x9F\x98\x80";
  Value v(emoji);
  string result = v.str();
  EXPECT_NE(string::npos, result.find("\\u++++"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
