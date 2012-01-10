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

TEST(AMFValueTest, TestScalarValueOutput)
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

TEST(AMFValueTest, TestAssociativeArrayOutput)
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

TEST(AMFValueTest, TestObjectOutput)
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

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
