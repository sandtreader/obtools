//==========================================================================
// ObTools::Redis: test-protocol.cc
//
// Test harness for RESP2 encoding and decoding
//
// Copyright (c) 2026 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-redis.h"
#include <sstream>

namespace {

using namespace std;
using namespace ObTools;

bool read(const string& resp, Redis::Value& value,
          const Redis::Limits& limits = {})
{
  istringstream iss(resp);
  return Redis::read_value(iss, value, limits);
}

TEST(RedisProtocolTest, TestEncodeCommand)
{
  EXPECT_EQ("*2\r\n$3\r\nGET\r\n$1\r\nx\r\n",
            Redis::encode_command({"GET", "x"}));
  EXPECT_EQ("*3\r\n$4\r\nSCAN\r\n$1\r\n0\r\n$0\r\n\r\n",
            Redis::encode_command({"SCAN", "0", ""}));
}

TEST(RedisProtocolTest, TestReadSimpleValues)
{
  Redis::Value v;
  ASSERT_TRUE(read("+OK\r\n", v));
  EXPECT_EQ(Redis::Value::Type::status, v.type);
  EXPECT_EQ("OK", v.str);

  ASSERT_TRUE(read("-WRONGTYPE Operation against a key\r\n", v));
  EXPECT_EQ(Redis::Value::Type::error, v.type);
  EXPECT_TRUE(v.is_error());
  EXPECT_EQ("WRONGTYPE Operation against a key", v.str);

  ASSERT_TRUE(read(":1793017118\r\n", v));
  EXPECT_EQ(Redis::Value::Type::integer, v.type);
  EXPECT_EQ(1793017118, v.integer);

  ASSERT_TRUE(read(":-42\r\n", v));
  EXPECT_EQ(-42, v.integer);

  ASSERT_TRUE(read("$-1\r\n", v));
  EXPECT_EQ(Redis::Value::Type::nil, v.type);
  EXPECT_TRUE(v.is_nil());
}

TEST(RedisProtocolTest, TestReadBulkWithCRLFInside)
{
  // Bulk strings are length delimited, so may hold anything
  Redis::Value v;
  ASSERT_TRUE(read("$7\r\na\r\nb c\r\r\n", v));
  EXPECT_EQ(Redis::Value::Type::bulk, v.type);
  EXPECT_EQ("a\r\nb c\r", v.str);

  ASSERT_TRUE(read(string("$3\r\na\0b\r\n", 10), v));
  EXPECT_EQ(string("a\0b", 3), v.str);

  ASSERT_TRUE(read("$0\r\n\r\n", v));
  EXPECT_EQ(Redis::Value::Type::bulk, v.type);
  EXPECT_EQ("", v.str);
}

TEST(RedisProtocolTest, TestReadNestedArray)
{
  // SCAN reply: cursor and keys
  Redis::Value v;
  ASSERT_TRUE(read("*2\r\n$2\r\n17\r\n*2\r\n$5\r\nmac_a\r\n$5\r\nmac_b\r\n",
                   v));
  ASSERT_EQ(Redis::Value::Type::array, v.type);
  ASSERT_EQ(2u, v.elements.size());
  EXPECT_EQ("17", v.elements[0].str);
  ASSERT_EQ(Redis::Value::Type::array, v.elements[1].type);
  ASSERT_EQ(2u, v.elements[1].elements.size());
  EXPECT_EQ("mac_b", v.elements[1].elements[1].str);

  ASSERT_TRUE(read("*-1\r\n", v));
  EXPECT_TRUE(v.is_nil());

  ASSERT_TRUE(read("*0\r\n", v));
  EXPECT_EQ(Redis::Value::Type::array, v.type);
  EXPECT_TRUE(v.elements.empty());
}

TEST(RedisProtocolTest, TestReadCommand)
{
  // A command is an array of bulk strings, so can be read too
  Redis::Value v;
  ASSERT_TRUE(read(Redis::encode_command({"HGETALL", "mac_x"}), v));
  ASSERT_EQ(2u, v.elements.size());
  EXPECT_EQ("HGETALL", v.elements[0].str);
  EXPECT_EQ("mac_x", v.elements[1].str);
}

TEST(RedisProtocolTest, TestReadSeveralFromOneStream)
{
  // Pipelined replies
  istringstream iss(":1\r\n$1\r\nx\r\n-ERR no\r\n");
  Redis::Value a, b, c, d;
  ASSERT_TRUE(Redis::read_value(iss, a));
  ASSERT_TRUE(Redis::read_value(iss, b));
  ASSERT_TRUE(Redis::read_value(iss, c));
  EXPECT_EQ(1, a.integer);
  EXPECT_EQ("x", b.str);
  EXPECT_TRUE(c.is_error());
  EXPECT_FALSE(Redis::read_value(iss, d));
}

TEST(RedisProtocolTest, TestReadMalformed)
{
  Redis::Value v;
  EXPECT_FALSE(read("", v));
  EXPECT_FALSE(read("?what\r\n", v));
  EXPECT_FALSE(read("+OK\n", v));               // No CR
  EXPECT_FALSE(read(":abc\r\n", v));
  EXPECT_FALSE(read(":\r\n", v));
  EXPECT_FALSE(read("$5\r\nab\r\n", v));        // Truncated bulk
  EXPECT_FALSE(read("$2\r\nabcd\r\n", v));      // No CRLF after bulk
  EXPECT_FALSE(read("*2\r\n:1\r\n", v));        // Truncated array
  EXPECT_FALSE(read("$x\r\n", v));
  EXPECT_FALSE(read("$-2\r\n", v));
  EXPECT_FALSE(read("*-2\r\n", v));
}

TEST(RedisProtocolTest, TestReadLimits)
{
  Redis::Limits limits;
  limits.max_bulk = 4;
  limits.max_elements = 2;
  limits.max_depth = 2;

  Redis::Value v;
  EXPECT_TRUE(read("$4\r\nabcd\r\n", v, limits));
  // Rejected on the length, before reading (or allocating) it
  EXPECT_FALSE(read("$5\r\nabcde\r\n", v, limits));
  EXPECT_FALSE(read("$999999999999\r\n", v));   // Over the default

  EXPECT_TRUE(read("*2\r\n:1\r\n:2\r\n", v, limits));
  EXPECT_FALSE(read("*3\r\n:1\r\n:2\r\n:3\r\n", v, limits));

  EXPECT_TRUE(read("*1\r\n*1\r\n:1\r\n", v, limits));
  EXPECT_FALSE(read("*1\r\n*1\r\n*1\r\n:1\r\n", v, limits));
}

TEST(RedisProtocolTest, TestHashFromValue)
{
  Redis::Value v;
  ASSERT_TRUE(read("*4\r\n$2\r\nip\r\n$11\r\n10.19.64.38\r\n"
                   "$12\r\nget-messages\r\n$10\r\n1793013556\r\n", v));
  map<string, string> fields;
  ASSERT_TRUE(Redis::hash_from_value(v, fields));
  EXPECT_EQ(2u, fields.size());
  EXPECT_EQ("10.19.64.38", fields["ip"]);
  EXPECT_EQ("1793013556", fields["get-messages"]);

  // Empty hash (key not set)
  ASSERT_TRUE(read("*0\r\n", v));
  fields.clear();
  EXPECT_TRUE(Redis::hash_from_value(v, fields));
  EXPECT_TRUE(fields.empty());

  // Not a hash
  ASSERT_TRUE(read("-WRONGTYPE Operation against a key\r\n", v));
  EXPECT_FALSE(Redis::hash_from_value(v, fields));
  ASSERT_TRUE(read("*1\r\n$2\r\nip\r\n", v));
  EXPECT_FALSE(Redis::hash_from_value(v, fields));
  ASSERT_TRUE(read("*2\r\n$2\r\nip\r\n:1\r\n", v));
  EXPECT_FALSE(Redis::hash_from_value(v, fields));
}

} // anonymous namespace

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
