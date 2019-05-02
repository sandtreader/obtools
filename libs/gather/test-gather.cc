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
  char data[] = "Hello, world!";
  const string expected(&data[0], strlen(data));
  Gather::Segment& seg = buffer.add(reinterpret_cast<unsigned char *>(&data[0]),
                                    strlen(data));
  ASSERT_EQ(expected.size(), buffer.get_length());
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
  uint32_t n = 0xDEADBEEF;
  const uint32_t expected = n;
  Gather::Segment &seg = buffer.insert(reinterpret_cast<Gather::data_t *>(&n),
                                       sizeof(n));
  ASSERT_EQ(sizeof(expected), buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(sizeof(expected), seg.length);
  for (unsigned i = 0; i < sizeof(expected); ++i)
    ASSERT_EQ((expected >> (i * 8)) & 0xff, seg.data[i]) << "Where i = " << i;
}

TEST(GatherTest, TestInsertBetween)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  const string expected_str(&data[0], strlen(data));
  uint32_t n = 0x01234567;
  const uint32_t expected_num(n);
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));
  buffer.insert(reinterpret_cast<Gather::data_t *>(&n), sizeof(n), 1);
  const Gather::Segment *segments = buffer.get_segments();

  ASSERT_EQ(expected_str.size() * 2 + sizeof(expected_num),
            buffer.get_length());
  ASSERT_LE(3, buffer.get_size());
  ASSERT_EQ(3, buffer.get_count());
  ASSERT_TRUE(segments);

  const Gather::Segment &seg1 = segments[0];
  ASSERT_EQ(expected_str.size(), seg1.length);
  ASSERT_EQ(expected_str,
            string(reinterpret_cast<char *>(seg1.data), seg1.length));

  const Gather::Segment &seg2 = segments[1];
  ASSERT_EQ(sizeof(expected_num), seg2.length);
  for (unsigned i = 0; i < sizeof(expected_num); ++i)
    ASSERT_EQ((expected_num >> (i * 8)) & 0xff, seg2.data[i])
             << "Where i = " << i;

  const Gather::Segment &seg3 = segments[2];
  ASSERT_EQ(expected_str.size(), seg3.length);
  ASSERT_EQ(expected_str,
            string(reinterpret_cast<char *>(seg3.data), seg3.length));
}

TEST(GatherTest, TestSimpleLimit)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  const unsigned int chop(8);
  const string expected(&data[0], strlen(data) - chop);
  Gather::Segment& seg = buffer.add(
                              reinterpret_cast<Gather::data_t *>(&data[0]),
                              strlen(data));
  buffer.limit(buffer.get_length() - chop);
  ASSERT_EQ(expected.size(), buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(expected, string(reinterpret_cast<char *>(seg.data), seg.length));
}

TEST(GatherTest, TestSimpleConsume)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  const unsigned int chop(7);
  const string expected(&data[chop], strlen(data) - chop);
  Gather::Segment& seg = buffer.add(
                              reinterpret_cast<Gather::data_t *>(&data[0]),
                              strlen(data));
  buffer.consume(chop);
  ASSERT_EQ(expected.size(), buffer.get_length());
  ASSERT_LE(1, buffer.get_size());
  ASSERT_EQ(1, buffer.get_count());
  ASSERT_EQ(expected, string(reinterpret_cast<char *>(seg.data), seg.length));
}

TEST(GatherTest, TestCopy)
{
  Gather::Buffer buffer(0);
  char one[] = "xHell";
  char two[] = "o, wo";
  char three[] = "rld!x";
  const string expected = string(&one[1], strlen(one) - 1)
                        + string(&two[0], strlen(two))
                        + string(&three[0], strlen(three) - 1);
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  string actual;
  actual.resize(expected.size());
  unsigned int copied = buffer.copy(
      reinterpret_cast<Gather::data_t *>(const_cast<char *>(actual.c_str())),
      1, expected.size());

  ASSERT_EQ(expected.size(), copied);
  ASSERT_EQ(expected, actual);
}

TEST(GatherTest, TestAddFromBuffer)
{
  Gather::Buffer buffer1(0);
  char one[] = "xHell";
  const string one_str(&one[0], strlen(one));
  char two[] = "o, wo";
  const string two_str(&two[0], strlen(two));
  char three[] = "rld!x";
  const string three_str(&three[0], strlen(three));
  buffer1.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer1.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer1.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  Gather::Buffer buffer2(0);
  char data[] = "Hello";
  const string hello(&data[0], strlen(data));
  buffer2.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));
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
  ASSERT_EQ(two_str.substr(1).size(), segment2.length);
  ASSERT_EQ(two_str.substr(1),
            string(reinterpret_cast<char *>(segment2.data), segment2.length));

  const Gather::Segment& segment3 = segments[2];
  ASSERT_EQ(three_str.substr(0, 4).size(), segment3.length);
  ASSERT_EQ(three_str.substr(0, 4),
            string(reinterpret_cast<char *>(segment3.data), segment3.length));
}

TEST(GatherTest, TestAddBuffer)
{
  Gather::Buffer buffer1(0);
  char one[] = "Hello";
  const string one_str(&one[0], strlen(one));
  char two[] = ", wo";
  const string two_str(&two[0], strlen(two));
  char three[] = "rld!";
  const string three_str(&three[0], strlen(three));
  buffer1.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));

  Gather::Buffer buffer2(0);
  Gather::Segment& segment = buffer2.add(strlen(two));
  memcpy(segment.data, &two[0], segment.length);
  buffer2.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  buffer1.add(buffer2);

  const Gather::Segment *segments = buffer1.get_segments();
  ASSERT_LE(3, buffer1.get_size());
  ASSERT_EQ(3, buffer1.get_count());
  ASSERT_TRUE(segments);

  const Gather::Segment& segment1 = segments[0];
  ASSERT_EQ(one_str.size(), segment1.length);
  ASSERT_EQ(one_str,
            string(reinterpret_cast<char *>(segment1.data), segment1.length));

  const Gather::Segment& segment2 = segments[1];
  ASSERT_EQ(two_str.size(), segment2.length);
  ASSERT_EQ(two_str,
            string(reinterpret_cast<char *>(segment2.data), segment2.length));

  const Gather::Segment& segment3 = segments[2];
  ASSERT_EQ(three_str.size(), segment3.length);
  ASSERT_EQ(three_str,
            string(reinterpret_cast<char *>(segment3.data), segment3.length));
}

TEST(GatherTest, TestIteratorLoop)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  const string one_str(&one[0], strlen(one));
  char two[] = ", wo";
  const string two_str(&two[0], strlen(two));
  char three[] = "rld!";
  const string three_str(&three[0], strlen(three));
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  const string expect = one_str + two_str + three_str;
  string actual;

  for (Gather::Buffer::iterator it = buffer.begin(); it != buffer.end(); ++it)
    actual += reinterpret_cast<char&>(*it);

  ASSERT_EQ(expect.size(), actual.size());
  ASSERT_EQ(expect, actual);
}

TEST(GatherTest, TestIteratorAdd)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  const string one_str(&one[0], strlen(one));
  char two[] = ", wo";
  const string two_str(&two[0], strlen(two));
  char three[] = "rld!";
  const string three_str(&three[0], strlen(three));
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  const string expect = one_str + two_str + three_str;

  Gather::Buffer::iterator it = buffer.begin();
  it += 7;

  ASSERT_EQ(expect[7], *it);
}

TEST(GatherTest, TestIteratorSub)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  const string one_str(&one[0], strlen(one));
  char two[] = ", wo";
  const string two_str(&two[0], strlen(two));
  char three[] = "rld!";
  const string three_str(&three[0], strlen(three));
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  const string expect = one_str + two_str + three_str;

  Gather::Buffer::iterator it = buffer.end();
  it -= 6;

  ASSERT_EQ(expect[7], *it);

  it -= 1;

  ASSERT_EQ(expect[6], *it);
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
  char data[] = "Hello, world!";

  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Segment& seg = buffer.add(16);
  for (unsigned int i = 0; i < seg.length; ++i)
    seg.data[i] = i;

  uint32_t n = 0xDEADBEEF;
  buffer.insert(reinterpret_cast<Gather::data_t *>(&n), sizeof(n));

  uint32_t n2 = 0x01234567;
  buffer.insert(reinterpret_cast<Gather::data_t *>(&n2), sizeof(n2), 2);

  buffer.limit(buffer.get_length() - 8);

  buffer.consume(5);

  ostringstream actual;
  buffer.dump(actual, true);

  ASSERT_EQ(actual.str(), expect.str());
}

#if !defined(PLATFORM_WINDOWS)
TEST(GatherTest, TestFill)
{
  Gather::Buffer buffer(1);

  char data[] = "Hello, world!";

  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Segment& seg = buffer.add(16);
  for (unsigned int i = 0; i < seg.length; ++i)
    seg.data[i] = i;

  uint32_t n = 0xDEADBEEF;
  buffer.insert(reinterpret_cast<Gather::data_t *>(&n), sizeof(n));

  uint32_t n2 = 0x01234567;
  buffer.insert(reinterpret_cast<Gather::data_t *>(&n2), sizeof(n), 2);

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

TEST(GatherTest, TestGetFlatDataSingleSegment)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::data_t buf[4];
  Gather::data_t *p = buffer.get_flat_data(0, 4, buf);
  ASSERT_NE(p, buf) << "Single segment get_flat_data used temporary buffer!\n";
  ASSERT_EQ(string(reinterpret_cast<char *>(p), 4), "Hell");
}

TEST(GatherTest, TestGetFlatDataMultiSegment)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = ", wo";
  char three[] = "rld!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  Gather::data_t buf[7];
  Gather::data_t *p = buffer.get_flat_data(3, 7, buf);
  ASSERT_EQ(p, buf) << "Multi-segment get_flat_data didn't use temporary buffer!\n";
  ASSERT_EQ(string(reinterpret_cast<char *>(p), 7), "lo, wor");
}

TEST(GatherTest, TestGetFlatDataOffEndFails)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = ", wo";
  char three[] = "rld!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  Gather::data_t buf[7];
  Gather::data_t *p = buffer.get_flat_data(7, 7, buf);
  ASSERT_FALSE(p) << "get_flat_data off the end didn't fail!\n";
}

TEST(GatherTest, TestReplaceSingleSegment)
{
  Gather::Buffer buffer(0);

  // Need to ensure it's owned data, not referenced
  memcpy(buffer.add(13).data, "Hello, world!", 13);
  buffer.replace(0, reinterpret_cast<const Gather::data_t *>("Salut"), 5);

  Gather::data_t buf[13];
  Gather::data_t *p = buffer.get_flat_data(0, 13, buf);
  ASSERT_EQ(string(reinterpret_cast<char *>(p), 13), "Salut, world!");
}

TEST(GatherTest, TestReplaceMultiSegment)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = ", wo";
  char three[] = "rld!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  buffer.replace(4, reinterpret_cast<const Gather::data_t *>(" freezeth"), 9);

  Gather::data_t buf[13];
  Gather::data_t *p = buffer.get_flat_data(0, 13, buf);
  ASSERT_EQ(string(reinterpret_cast<char *>(p), 13), "Hell freezeth");
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
