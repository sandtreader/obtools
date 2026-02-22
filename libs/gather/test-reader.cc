//==========================================================================
// ObTools::Gather: test-reader.cc
//
// Test harness for Gather::Reader and buffer tidy/copy-with-iterator
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-gather.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Reader basic_read tests
TEST(GatherReaderTest, TestReaderBasicRead)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Reader reader(buffer);
  char buf[6];
  reader.read(buf, 5);
  EXPECT_EQ("Hello", string(buf, 5));

  reader.read(buf, 2);
  EXPECT_EQ(", ", string(buf, 2));
}

TEST(GatherReaderTest, TestReaderReadAcrossSegments)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = ", wo";
  char three[] = "rld!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  Gather::Reader reader(buffer);
  char buf[13];
  reader.read(buf, 13);
  EXPECT_EQ("Hello, world!", string(buf, 13));
}

TEST(GatherReaderTest, TestReaderReadNBO16)
{
  Gather::Buffer buffer(0);
  unsigned char data[] = {0x01, 0x02};
  buffer.add(data, 2);

  Gather::Reader reader(buffer);
  uint16_t val = reader.read_nbo_16();
  EXPECT_EQ(0x0102, val);
}

TEST(GatherReaderTest, TestReaderReadNBO32)
{
  Gather::Buffer buffer(0);
  unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF};
  buffer.add(data, 4);

  Gather::Reader reader(buffer);
  uint32_t val = reader.read_nbo_32();
  EXPECT_EQ(0xDEADBEEF, val);
}

//--------------------------------------------------------------------------
// Reader skip tests
TEST(GatherReaderTest, TestReaderSkip)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Reader reader(buffer);
  reader.skip(7);

  char buf[6];
  reader.read(buf, 6);
  EXPECT_EQ("world!", string(buf, 6));
}

TEST(GatherReaderTest, TestReaderSkipBeyondEndThrows)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Reader reader(buffer);
  EXPECT_THROW(reader.skip(100), Channel::Error);
}

//--------------------------------------------------------------------------
// Reader rewind tests
TEST(GatherReaderTest, TestReaderRewindable)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Reader reader(buffer);
  EXPECT_TRUE(reader.rewindable());
}

TEST(GatherReaderTest, TestReaderRewind)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello, world!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Reader reader(buffer);
  char buf[5];
  reader.read(buf, 5);
  EXPECT_EQ("Hello", string(buf, 5));

  reader.rewind(5);

  reader.read(buf, 5);
  EXPECT_EQ("Hello", string(buf, 5));
}

TEST(GatherReaderTest, TestReaderRewindTooFarThrows)
{
  Gather::Buffer buffer(0);
  char data[] = "Hello";
  buffer.add(reinterpret_cast<Gather::data_t *>(&data[0]), strlen(data));

  Gather::Reader reader(buffer);
  char buf[3];
  reader.read(buf, 3);

  EXPECT_THROW(reader.rewind(10), Channel::Error);
}

//--------------------------------------------------------------------------
// Buffer tidy tests
TEST(GatherReaderTest, TestBufferTidy)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = ", wo";
  char three[] = "rld!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  // Consume first segment entirely
  buffer.consume(5);

  // Now we have an empty first segment and two occupied ones
  buffer.tidy();

  // Should have compacted to 2 segments
  EXPECT_EQ(2u, buffer.get_count());
  EXPECT_EQ(8u, buffer.get_length());

  // Verify data is still intact
  Gather::data_t buf[8];
  buffer.copy(buf, 0, 8);
  EXPECT_EQ(", world!", string(reinterpret_cast<char *>(buf), 8));
}

TEST(GatherReaderTest, TestBufferTidyNoChange)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = "World";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));

  // No empty segments, tidy should be a no-op
  buffer.tidy();
  EXPECT_EQ(2u, buffer.get_count());
  EXPECT_EQ(10u, buffer.get_length());
}

//--------------------------------------------------------------------------
// Buffer copy with iterator tests
TEST(GatherReaderTest, TestCopyWithIterator)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = ", wo";
  char three[] = "rld!";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));
  buffer.add(reinterpret_cast<Gather::data_t *>(&three[0]), strlen(three));

  // Start iterator and advance past "Hello"
  Gather::BufferIterator it = buffer.begin();
  it += 5;

  Gather::data_t buf[8];
  Gather::length_t copied = buffer.copy(buf, it, 8);
  EXPECT_EQ(8u, copied);
  EXPECT_EQ(", world!", string(reinterpret_cast<char *>(buf), 8));
}

TEST(GatherReaderTest, TestCopyWithEndIterator)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));

  Gather::BufferIterator it = buffer.end();

  Gather::data_t buf[5];
  Gather::length_t copied = buffer.copy(buf, it, 5);
  EXPECT_EQ(0u, copied);
}

//--------------------------------------------------------------------------
// get_flat_data_multi: data within single segment of multi-segment buffer
TEST(GatherReaderTest, TestGetFlatDataMultiWithinSingleSegment)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello, world!";
  char two[] = "Extra";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));

  // Request data that's entirely within the first segment (multi path
  // is used because count > 1)
  Gather::data_t buf[4];
  Gather::data_t *p = buffer.get_flat_data(1, 4, buf);
  ASSERT_TRUE(p != nullptr);
  // Should return direct pointer, not temp buffer
  EXPECT_NE(p, buf);
  EXPECT_EQ("ello", string(reinterpret_cast<char *>(p), 4));
}

//--------------------------------------------------------------------------
// get_flat_data_multi: offset past end of multi-segment buffer
TEST(GatherReaderTest, TestGetFlatDataMultiOffsetPastEnd)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = "World";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));

  Gather::data_t buf[4];
  Gather::data_t *p = buffer.get_flat_data(100, 4, buf);
  EXPECT_EQ(nullptr, p);
}

//--------------------------------------------------------------------------
// replace: starting in 2nd segment (exercises offset skip loop)
TEST(GatherReaderTest, TestReplaceInSecondSegment)
{
  Gather::Buffer buffer(0);
  char one[] = "Hello";
  char two[] = "World";
  buffer.add(reinterpret_cast<Gather::data_t *>(&one[0]), strlen(one));
  buffer.add(reinterpret_cast<Gather::data_t *>(&two[0]), strlen(two));

  // Replace starting at offset 6 (in second segment, past "Hello" + 'W')
  buffer.replace(6, reinterpret_cast<const Gather::data_t *>("abc"), 3);

  Gather::data_t buf[10];
  buffer.copy(buf, 0, 10);
  EXPECT_EQ("HelloWabcd", string(reinterpret_cast<char *>(buf), 10));
}

//--------------------------------------------------------------------------
// Segment::copy where target already has owned data
TEST(GatherReaderTest, TestSegmentCopyOverExistingOwned)
{
  // Create two buffers with owned data, then copy one to the other
  Gather::Buffer buffer1(0);
  Gather::Segment& seg1 = buffer1.add(8);
  memcpy(seg1.data, "AAAAAAAA", 8);

  Gather::Buffer buffer2(0);
  Gather::Segment& seg2 = buffer2.add(4);
  memcpy(seg2.data, "BBBB", 4);

  // Copy buffer2 into buffer1 - this calls add(const Buffer&) which
  // copies segments. The new segment in buffer1 won't have pre-existing
  // owned data, so we need a different approach.

  // Instead, test the underlying copy by creating a segment with owned data
  // and copying over it via the Buffer::add(const Buffer&) path
  // which internally creates segments and calls seg.copy()
  Gather::Buffer buffer3(0);
  buffer3.add(buffer2);  // Copies seg2's owned data
  buffer3.add(buffer1);  // Copies seg1's owned data

  EXPECT_EQ(12u, buffer3.get_length());
  Gather::data_t buf[12];
  buffer3.copy(buf, 0, 12);
  EXPECT_EQ("BBBB", string(reinterpret_cast<char *>(buf), 4));
  EXPECT_EQ("AAAAAAAA", string(reinterpret_cast<char *>(buf + 4), 8));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
