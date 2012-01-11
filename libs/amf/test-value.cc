//==========================================================================
// ObTools::AMF: test-value.cc
//
// Test harness for value read/write
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-amf.h"
#include <sstream>

using namespace std;
using namespace ObTools;

TEST(AMFValueTest, TestScalarValueLogging)
{
  AMF::Value value(AMF::Value::ARRAY);
  value.add(AMF::Value(AMF::Value::UNDEFINED));
  value.add(AMF::Value(AMF::Value::NULLV));
  value.add(AMF::Value(AMF::Value::TRUE));
  value.add(AMF::Value(AMF::Value::FALSE));
  value.add(AMF::Value(AMF::Value::INTEGER, 1234));
  value.add(AMF::Value(AMF::Value::DOUBLE, 3.1415926));
  value.add(AMF::Value(AMF::Value::STRING, "Hello"));
  value.add(AMF::Value(AMF::Value::XML_DOC, "<doc/>"));
  value.add(AMF::Value(AMF::Value::DATE, -9.2252e10));
  value.add(AMF::Value(AMF::Value::XML, "<doc2/>"));
  value.add(AMF::Value(AMF::Value::BYTE_ARRAY, "DATA"));

  ostringstream oss;
  oss << value;
  string result = oss.str();

  string expected =
    "array dense(11):\n"
    "  undefined\n"
    "  null\n"
    "  true\n"
    "  false\n"
    "  integer: 1234\n"
    "  double: 3.14159\n"
    "  string: 'Hello'\n"
    "  xml-doc:\n"
    "<doc/>\n"
    "  double: -9.2252e+10 (1967-01-29T06:26:40Z)\n"
    "  xml:\n"
    "<doc2/>\n"
    "  byte-array: 4 bytes\n"
    "0000: 44415441                            | DATA\n"
    "\n";

  ASSERT_EQ(expected, result);
}

TEST(AMFValueTest, TestAssociativeArrayLogging)
{
  AMF::Value value(AMF::Value::ARRAY);
  value.set("foo", AMF::Value(AMF::Value::INTEGER, 99));
  value.set("bar", AMF::Value(AMF::Value::STRING, "Hello"));

  ostringstream oss;
  oss << value;
  string result = oss.str();

  string expected =
    "array associative:\n"
    "  bar = string: 'Hello'\n"
    "  foo = integer: 99\n";

  ASSERT_EQ(expected, result);
}

TEST(AMFValueTest, TestObjectLogging)
{
  AMF::Value value(AMF::Value::OBJECT);
  value.set("foo", AMF::Value(AMF::Value::INTEGER, 99));
  value.set("bar", AMF::Value(AMF::Value::STRING, "Hello"));

  ostringstream oss;
  oss << value;
  string result = oss.str();

  string expected =
    "object:\n"
    "  bar = string: 'Hello'\n"
    "  foo = integer: 99\n";

  ASSERT_EQ(expected, result);
}

TEST(AMFValueTest, TestReadingAMF0Number)
{
  unsigned char buf[9];
  buf[0] = 0x00;
  buf[1] = 0x40;   // IEEE double 0x40 00 00 00 00 00 00 00 = 2.0
  memset(buf+2, 0, 7);

  Channel::BlockReader chan(buf, 9);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 9);
  EXPECT_EQ(value.type, AMF::Value::DOUBLE);
  EXPECT_EQ(value.d, 2.0);
}

TEST(AMFValueTest, TestReadingAMF0True)
{
  unsigned char buf[2];
  buf[0] = 0x01;
  buf[1] = 0xFF;

  Channel::BlockReader chan(buf, 2);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 2);
  EXPECT_EQ(value.type, AMF::Value::TRUE);
}

TEST(AMFValueTest, TestReadingAMF0False)
{
  unsigned char buf[2];
  buf[0] = 0x01;
  buf[1] = 0x00;

  Channel::BlockReader chan(buf, 2);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 2);
  EXPECT_EQ(value.type, AMF::Value::FALSE);
}

TEST(AMFValueTest, TestReadingAMF0String)
{
  unsigned char buf[8];
  buf[0] = 0x02;
  buf[1] = 0x00;
  buf[2] = 0x05;
  memcpy(buf+3, "Hello", 5);

  Channel::BlockReader chan(buf, 8);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 8);
  EXPECT_EQ(value.type, AMF::Value::STRING);
  EXPECT_EQ(value.text, "Hello");
}

TEST(AMFValueTest, TestReadingAMF0Object)
{
  unsigned char buf[17];
  buf[0] = 0x03;
  buf[1] = 0x00;
  buf[2] = 0x03;
  memcpy(buf+3, "foo", 3);
  buf[6] = 0x06;  // undefined

  buf[7] = 0x00;
  buf[8] = 0x03;
  memcpy(buf+9, "bar", 3);
  buf[12] = 0x01;
  buf[13] = 0xFF;  // true

  buf[14] = 0x00;
  buf[15] = 0x00;
  buf[16] = 0x09;  // end

  Channel::BlockReader chan(buf, 17);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 17);
  EXPECT_EQ(value.type, AMF::Value::OBJECT);
  EXPECT_EQ(value.assoc_array.size(), 2);
  EXPECT_EQ(value.assoc_array["foo"].type, AMF::Value::UNDEFINED);
  EXPECT_EQ(value.assoc_array["bar"].type, AMF::Value::TRUE);
}

TEST(AMFValueTest, TestReadingAMF0Null)
{
  unsigned char c = 0x05;
  Channel::BlockReader chan(&c, 1);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 1);
  EXPECT_EQ(value.type, AMF::Value::NULLV);
}

TEST(AMFValueTest, TestReadingAMF0Undefined)
{
  unsigned char c = 0x06;
  Channel::BlockReader chan(&c, 1);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 1);
  EXPECT_EQ(value.type, AMF::Value::UNDEFINED);
}

TEST(AMFValueTest, TestReadingAMF0ECMAArray)
{
  unsigned char buf[18];
  buf[0] = 0x08;
  buf[1] = 0x00;
  buf[2] = 0x00;
  buf[3] = 0x00;
  buf[4] = 0x02;  // 2 values

  buf[5] = 0x00;
  buf[6] = 0x03;
  memcpy(buf+7, "foo", 3);
  buf[10] = 0x06;  // undefined

  buf[11] = 0x00;
  buf[12] = 0x03;
  memcpy(buf+13, "bar", 3);
  buf[16] = 0x01;
  buf[17] = 0xFF;  // true

  Channel::BlockReader chan(buf, 18);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 18);
  EXPECT_EQ(value.type, AMF::Value::ARRAY);
  EXPECT_EQ(value.assoc_array.size(), 2);
  EXPECT_EQ(value.assoc_array["foo"].type, AMF::Value::UNDEFINED);
  EXPECT_EQ(value.assoc_array["bar"].type, AMF::Value::TRUE);
}

TEST(AMFValueTest, TestReadingAMF0StrictArray)
{
  unsigned char buf[16];
  buf[0] = 0x0A;
  buf[1] = 0x00;
  buf[2] = 0x00;
  buf[3] = 0x00;
  buf[4] = 0x02;  // 2 values

  buf[5] = 0x06;  // undefined
  buf[6] = 0x01;
  buf[7] = 0xFF;  // true

  Channel::BlockReader chan(buf, 16);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 8);
  EXPECT_EQ(value.type, AMF::Value::ARRAY);
  EXPECT_EQ(value.dense_array.size(), 2);
  EXPECT_EQ(value.dense_array[0].type, AMF::Value::UNDEFINED);
  EXPECT_EQ(value.dense_array[1].type, AMF::Value::TRUE);
}

TEST(AMFValueTest, TestReadingAMF0Date)
{
  unsigned char buf[9];
  buf[0] = 0x0B;
  buf[1] = 0x40;   // IEEE double 0x40 00 00 00 00 00 00 00 = 2.0
  memset(buf+2, 0, 7);

  Channel::BlockReader chan(buf, 9);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 9);
  EXPECT_EQ(value.type, AMF::Value::DATE);
  EXPECT_EQ(value.d, 2.0);
}

TEST(AMFValueTest, TestReadingAMF0LongString)
{
  unsigned char buf[10];
  buf[0] = 0x0C;
  buf[1] = 0x00;
  buf[2] = 0x00;
  buf[3] = 0x00;
  buf[4] = 0x05;
  memcpy(buf+5, "Hello", 5);

  Channel::BlockReader chan(buf, 10);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 10);
  EXPECT_EQ(value.type, AMF::Value::STRING);
  EXPECT_EQ(value.text, "Hello");
}

TEST(AMFValueTest, TestReadingAMF0XMLDoc)
{
  unsigned char buf[9];
  buf[0] = 0x0F;
  buf[1] = 0x00;
  buf[2] = 0x06;
  memcpy(buf+3, "<doc/>", 6);

  Channel::BlockReader chan(buf, 9);
  AMF::Value value;
  ASSERT_NO_THROW(value.read(chan, AMF::FORMAT_AMF0));
  ASSERT_EQ(chan.get_offset(), 9);
  EXPECT_EQ(value.type, AMF::Value::XML_DOC);
  EXPECT_EQ(value.text, "<doc/>");
}

TEST(AMFValueTest, TestArrayGets)
{
  AMF::Value value(AMF::Value::ARRAY);
  value.set("foo", AMF::Value(AMF::Value::INTEGER, 99));
  value.set("bar", AMF::Value(AMF::Value::STRING, "Hello"));
  value.set("pi", AMF::Value(AMF::Value::DOUBLE, 3.1415926));
  value.set("true", AMF::Value(AMF::Value::TRUE));
  value.set("false", AMF::Value(AMF::Value::FALSE));

  // Integer
  AMF::Value::integer_t n;
  ASSERT_NO_THROW(n = value.get_integer("foo"));
  EXPECT_EQ(n, 99);

  // Double
  double pi;
  ASSERT_NO_THROW(pi = value.get_double("pi"));
  EXPECT_EQ(pi, 3.1415926);

  // String
  string s;
  ASSERT_NO_THROW(s = value.get_string("bar"));
  EXPECT_EQ(s, "Hello");

  // Booleans
  bool b;
  ASSERT_NO_THROW(b = value.get_boolean("true"));
  EXPECT_EQ(b, true);
  ASSERT_NO_THROW(b = value.get_boolean("false"));
  EXPECT_EQ(b, false);

  // Missing
  ASSERT_THROW(value.get("not-there"), AMF::TypeException);

  // Wrong type
  ASSERT_THROW(value.get_integer("pi"), AMF::TypeException);
  ASSERT_THROW(value.get_double("bar"), AMF::TypeException);
  ASSERT_THROW(value.get_string("foo"), AMF::TypeException);
  ASSERT_THROW(value.get_boolean("foo"), AMF::TypeException);
}

TEST(AMFValueTest, TestSillyGetsThrow)
{
  AMF::Value value(AMF::Value::FALSE);
  ASSERT_THROW(value.get("foo"), AMF::TypeException);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
