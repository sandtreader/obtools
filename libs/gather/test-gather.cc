//==========================================================================
// ObTools::Gather: test-buffer.cc
//
// Test harness for gather buffer library
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gather.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(GatherTest, TestSimpleAdd)
{
  Gather::Buffer buffer(0);
  const string data("Hello, world!");
  Gather::Segment& seg = buffer.add((Gather::data_t *)data.c_str(),
                                    data.size());
  ASSERT_EQ(data.size(), buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(data, string(reinterpret_cast<char *>(seg.data), seg.length));
}

TEST(GatherTest, TestInternalAdd)
{
  Gather::Buffer buffer(0);
  Gather::Segment& seg = buffer.add(16);
  for (unsigned int i = 0; i < seg.length; ++i)
    seg.data[i] = i;
  ASSERT_EQ(16, buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  for (unsigned int i = 0; i < seg.length; ++i)
    ASSERT_EQ(i, seg.data[i]) << "Where i = " << i;
}

TEST(GatherTest, TestSimpleInsert)
{
  Gather::Buffer buffer(0);
  const uint32_t n = 0xDEADBEEF;
  Gather::Segment &seg = buffer.insert((Gather::data_t *)&n, sizeof(n));
  ASSERT_EQ(sizeof(n), buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(sizeof(n), seg.length);
  for (unsigned i = 0; i < sizeof(n); ++i)
    ASSERT_EQ((n >> (i * 8)) & 0xff, seg.data[i]) << "Where i = " << i;
}

TEST(GatherTest, TestInsertBetween)
{
  Gather::Buffer buffer(0);
  const string data("Hello, world!");
  const uint32_t n = 0x01234567;
  buffer.add((Gather::data_t *)data.c_str(), data.size());
  buffer.add((Gather::data_t *)data.c_str(), data.size());
  buffer.insert((Gather::data_t *)&n, sizeof(n), 1);
  const Gather::Segment *segments = buffer.get_segments();

  ASSERT_EQ(data.size() * 2 + sizeof(n), buffer.get_length());
  ASSERT_LE(3, buffer.get_size());
  ASSERT_EQ(3, buffer.get_count());
  ASSERT_TRUE(segments);

  const Gather::Segment &seg1 = segments[0];
  ASSERT_EQ(data.size(), seg1.length);
  ASSERT_EQ(data, string(reinterpret_cast<char *>(seg1.data), seg1.length));

  const Gather::Segment &seg2 = segments[1];
  ASSERT_EQ(sizeof(n), seg2.length);
  for (unsigned i = 0; i < sizeof(n); ++i)
    ASSERT_EQ((n >> (i * 8)) & 0xff, seg2.data[i]) << "Where i = " << i;

  const Gather::Segment &seg3 = segments[2];
  ASSERT_EQ(data.size(), seg3.length);
  ASSERT_EQ(data, string(reinterpret_cast<char *>(seg3.data), seg3.length));
}

TEST(GatherTest, TestSimpleLimit)
{
  Gather::Buffer buffer(0);
  const string data("Hello, world!");
  const unsigned int chop(8);
  Gather::Segment& seg = buffer.add((Gather::data_t *)data.c_str(),
                                    data.size());
  buffer.limit(buffer.get_length() - chop);
  ASSERT_EQ(data.size() - chop, buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(data.substr(0, data.size() - chop),
            string(reinterpret_cast<char *>(seg.data), seg.length));
}

TEST(GatherTest, TestSimpleConsume)
{
  Gather::Buffer buffer(0);
  const string data("Hello, world!");
  const unsigned int chop(7);
  Gather::Segment& seg = buffer.add((Gather::data_t *)data.c_str(),
                                    data.size());
  buffer.consume(chop);
  ASSERT_EQ(data.size() - chop, buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(data.substr(chop, data.size() - chop),
            string(reinterpret_cast<char *>(seg.data), seg.length));
}

TEST(GatherTest, TestCopy)
{
  Gather::Buffer buffer(0);
  const string one("xHell");
  const string two("o, wo");
  const string three("rld!x");
  buffer.add((Gather::data_t *)one.c_str(), one.size());
  buffer.add((Gather::data_t *)two.c_str(), two.size());
  buffer.add((Gather::data_t *)three.c_str(), three.size());

  const string expect = one.substr(1) + two + three.substr(0, 4);
  string actual;
  actual.resize(expect.size());
  unsigned int copied = buffer.copy(
      reinterpret_cast<Gather::data_t *>(const_cast<char *>(actual.c_str())),
      1, expect.size());

  ASSERT_EQ(expect.size(), copied);
  ASSERT_EQ(expect, actual);
}

TEST(GatherTest, TestAddFromBuffer)
{
  Gather::Buffer buffer1(0);
  const string one("xHell");
  const string two("o, wo");
  const string three("rld!x");
  buffer1.add((Gather::data_t *)one.c_str(), one.size());
  buffer1.add((Gather::data_t *)two.c_str(), two.size());
  buffer1.add((Gather::data_t *)three.c_str(), three.size());

  Gather::Buffer buffer2(0);
  const string hello("Hello");
  buffer2.add((Gather::data_t *)hello.c_str(), hello.size());
  buffer2.add(buffer1, 6, 8);
  const Gather::Segment *segments = buffer2.get_segments();

  ASSERT_LE(3, buffer2.get_size());
  ASSERT_EQ(3, buffer2.get_count());
  ASSERT_TRUE(segments);

  const Gather::Segment& segment1 = segments[0];
  ASSERT_EQ(hello.size(), segment1.length);
  ASSERT_EQ(hello,
            string(reinterpret_cast<char *>(segment1.data), segment1.length));

  const Gather::Segment& segment2 = segments[1];
  ASSERT_EQ(two.substr(1).size(), segment2.length);
  ASSERT_EQ(two.substr(1),
            string(reinterpret_cast<char *>(segment2.data), segment2.length));

  const Gather::Segment& segment3 = segments[2];
  ASSERT_EQ(three.substr(0, 4).size(), segment3.length);
  ASSERT_EQ(three.substr(0, 4),
            string(reinterpret_cast<char *>(segment3.data), segment3.length));
}

TEST(GatherTest, TestDump)
{
  Gather::Buffer buffer(1);
  ostringstream expect;
  expect << "Buffer (4/4):" << endl
         << "  0" << endl
         << "  12" << endl
         << "0000: 656c6c6f 2c20776f 726c6421          | ello, world!" << endl
         << "  4" << endl
         << "0000: 67452301                            | gE#." << endl
         << "* 8" << endl
         << "0000: 00010203 04050607                   | ........" << endl
         << "Total length 24" << endl;
  const string data("Hello, world!");

  buffer.add((Gather::data_t *)data.c_str(), data.size());

  Gather::Segment& seg = buffer.add(16);
  for (unsigned int i = 0; i < seg.length; ++i)
    seg.data[i] = i;

  const uint32_t n = 0xDEADBEEF;
  buffer.insert((Gather::data_t *)&n, 4);

  uint32_t n2 = 0x01234567;
  buffer.insert((Gather::data_t *)&n2, 4, 2);

  buffer.limit(buffer.get_length() - 8);

  buffer.consume(5);

  ostringstream actual;
  buffer.dump(actual, true);

  ASSERT_EQ(actual.str(), expect.str());
}

#if !defined(__WIN32__)
TEST(GatherTest, TestFill)
{
  Gather::Buffer buffer(1);

  const string data("Hello, world!");

  buffer.add((Gather::data_t *)data.c_str(), data.size());

  Gather::Segment& seg = buffer.add(16);
  for (unsigned int i = 0; i < seg.length; ++i)
    seg.data[i] = i;

  const uint32_t n = 0xDEADBEEF;
  buffer.insert((Gather::data_t *)&n, 4);

  uint32_t n2 = 0x01234567;
  buffer.insert((Gather::data_t *)&n2, 4, 2);

  buffer.limit(buffer.get_length() - 8);

  buffer.consume(5);

  struct iovec io[4];

  buffer.fill(io, 4);

  // Note: zero sized buffer is skipped
  ASSERT_EQ(12, io[0].iov_len);
  ASSERT_EQ(string("ello, world!"),
            string(reinterpret_cast<char *>(io[0].iov_base), io[0].iov_len));
  ASSERT_EQ(4, io[1].iov_len);
  ASSERT_EQ(string("gE#\x01"),
            string(reinterpret_cast<char *>(io[1].iov_base), io[1].iov_len));
  ASSERT_EQ(8, io[2].iov_len);
  ASSERT_EQ(string("\x00\x01\x02\x03\x04\x05\x06\x07", 8),
            string(reinterpret_cast<char *>(io[2].iov_base), io[2].iov_len));
}
#endif

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
