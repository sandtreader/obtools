//==========================================================================
// ObTools::File: test-stream.cc
//
// Test harness for file path library
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include "ot-text.h"
#include <iostream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

const string test_dir("../test.dump");

class StreamTest: public ::testing::Test
{
  virtual void SetUp()
  {
    File::Directory dir(test_dir);
    // Ensure test dir does not exist
    dir.erase();
    // Create test dir
    dir.ensure(true);
  }

  virtual void TearDown()
  {
    // Remove test dir
    File::Directory dir(test_dir);
    dir.erase();
  }
};

class MultiStreamTest: public StreamTest
{
};

class BufferedMultiStreamTest: public StreamTest
{
};

TEST_F(StreamTest, TestWriteBuffered)
{
  const string test_file(test_dir + "/write-buffered");
  const char data[] = "write-buffered";

  File::BufferedOutStream bos(test_file, 4);
  bos.write(&data[0], 3); // Due to a quirk of buffering setup in fstream,
                          // the first write will not buffer!
  bos.write(&data[3], 3); // So this one is the one we test..

  File::Path path(test_file);
  string str;
  ASSERT_TRUE(path.readable());
  ASSERT_TRUE(path.read_all(str));
  ASSERT_EQ(3, str.size());
  ASSERT_EQ((string{data, 3}), str);

  bos.close();
  str = "";
  ASSERT_TRUE(path.readable());
  ASSERT_TRUE(path.read_all(str));
  ASSERT_EQ(6, str.size());
  ASSERT_EQ((string{data, 6}), str);
}

TEST_F(StreamTest, TestBufferedWrite)
{
  const string test_file(test_dir + "/buffered-write");
  const char data[] = "buffered-write";

  {
    File::BufferedOutStream bos(test_file, 4);
    for (unsigned i = 0; i < sizeof(data); i += 2)
      bos.write(&data[i], 2);
  }

  File::Path path(test_file);
  ASSERT_TRUE(path.readable());
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_STREQ(data, str.c_str());
}

TEST_F(StreamTest, TestWriteLargerThanBuffer)
{
  const string test_file(test_dir + "/write-larger-than-buffer");
  const char data[] = "write-larger-than-buffer";

  {
    File::BufferedOutStream bos(test_file, 4);
    bos.write(data, 10);
    bos.write(&data[10], sizeof(data) - 10);
    bos.close();
  }

  File::Path path(test_file);
  ASSERT_TRUE(path.readable());
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_STREQ(data, str.c_str());
}

TEST_F(StreamTest, TestCurrentStreamPos)
{
  const string test_file(test_dir + "/current-stream-pos");
  const char data[] = "current-stream-pos";

  File::BufferedOutStream bos(test_file, 4);
  bos.write(data, 3);
  bos.write(data, 4);

  ASSERT_EQ(7, bos.tellp());
}

TEST_F(StreamTest, TestTruncation)
{
  const string test_file(test_dir + "/truncation");
  const char data[] = "truncation";

  File::Path path(test_file);
  path.write_all(data);

  ASSERT_TRUE(path.readable());
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_STREQ(data, str.c_str());

  File::BufferedOutStream bos(test_file, 4, ios_base::out | ios_base::trunc);
  bos.close();

  ASSERT_TRUE(path.readable());
  str.resize(0);
  ASSERT_TRUE(path.read_all(str));
  ASSERT_TRUE(str.empty());
}

TEST_F(StreamTest, TestClose)
{
  const string test_file(test_dir + "/close");
  const char data[] = "close";

  File::BufferedOutStream bos(test_file, 4);
  bos.write(data, sizeof(data));
  bos.close();

  ASSERT_FALSE(bos.is_open());
  File::Path path(test_file);
  ASSERT_TRUE(path.readable());
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_STREQ(data, str.c_str());
}

TEST_F(StreamTest, TestLargeBuffer)
{
  const string test_file(test_dir + "/large");
  vector<char> data(1234);

  File::BufferedOutStream bos(test_file, data.size());
  bos.write(&data[0], data.size() - 10);
  bos.close();

  File::Path path(test_file);
  ASSERT_TRUE(path.readable());
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_EQ(data.size() - 10, str.size());
}

TEST_F(StreamTest, TestTellpReturnsAmountWritten)
{
  const string test_file(test_dir + "/tellp");
  const char data[] = "some-text-as-a-test";

  File::BufferedOutStream bos(test_file, 4);
  bos.write(data, 10);
  bos.write(&data[10], sizeof(data) - 10);
  ASSERT_EQ(sizeof(data), bos.tellp());
}

TEST_F(MultiStreamTest, TestTellpReturnsAmountWritten)
{
  const string test_file(test_dir + "/tellp");
  const char data[] = "some-text-as-a-test";

  File::MultiOutStream mos;
  mos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  mos.write(data, sizeof(data));
  ASSERT_EQ(sizeof(data), mos.tellp());
}

TEST_F(BufferedMultiStreamTest, TestWriteBuffers)
{
  const auto num_files = 8;
  const auto buffer_size = 6;
  const auto data = string{"some-text-as-a-test"};
  auto files = vector<File::Path>{};
  for (auto i = 0; i < num_files; ++i)
    files.push_back(test_dir + "/test" + Text::itos(i));

  File::BufferedMultiOutStream mos{buffer_size};
  for (const auto& p: files)
    mos.open(p.c_str(), ios_base::out | ios_base::trunc);

  // Remember, first write isn't buffered
  auto str = string{};
  mos.write(data.c_str(), buffer_size);

  // Now testing can begin
  mos.write(data.c_str() + buffer_size, buffer_size - 1);
  for (auto& p: files)
  {
    str = "";
    ASSERT_TRUE(p.read_all(str));
    ASSERT_EQ(buffer_size, str.size());
  }

  mos.write(data.c_str() + buffer_size - 1, 1);
  for (auto& p: files)
  {
    str = "";
    ASSERT_TRUE(p.read_all(str));
    ASSERT_EQ(buffer_size * 2, str.size());
  }

  mos.write(data.c_str() + buffer_size * 2, data.size() - buffer_size * 2);
  mos.close();
  for (auto& p: files)
  {
    str = "";
    ASSERT_TRUE(p.read_all(str));
    ASSERT_EQ(data.size(), str.size());
  }
}

//--------------------------------------------------------------------------
// BufferedOutStream default constructor + open
TEST_F(StreamTest, TestBufferedOutStreamDefaultConstructor)
{
  const string test_file(test_dir + "/default-ctor");
  const char data[] = "hello";

  File::BufferedOutStream bos;
  EXPECT_FALSE(bos.is_open());
  bos.set_buffer_size(4);
  bos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  EXPECT_TRUE(bos.is_open());
  bos.write(data, sizeof(data));
  bos.close();
  EXPECT_FALSE(bos.is_open());

  File::Path path(test_file);
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_STREQ(data, str.c_str());
}

//--------------------------------------------------------------------------
// BufferedOutFileBuf resize
TEST_F(StreamTest, TestBufferedOutFileBufResize)
{
  const string test_file(test_dir + "/resize");
  const char data[] = "resize-test-data";

  File::BufferedOutStream bos(test_file, 8);
  EXPECT_EQ(8u, bos.get_buffer_size());

  // Resize larger
  bos.set_buffer_size(16);
  EXPECT_EQ(16u, bos.get_buffer_size());

  bos.write(data, sizeof(data));
  bos.close();

  File::Path path(test_file);
  string str;
  ASSERT_TRUE(path.read_all(str));
  ASSERT_STREQ(data, str.c_str());
}

//--------------------------------------------------------------------------
// MultiOutStream default constructor, is_open, close
TEST_F(MultiStreamTest, TestMultiOutStreamLifecycle)
{
  const string test_file1(test_dir + "/multi1");
  const string test_file2(test_dir + "/multi2");
  const char data[] = "multi-data";

  File::MultiOutStream mos;
  EXPECT_FALSE(mos.is_open());
  mos.open(test_file1.c_str(), ios_base::out | ios_base::trunc);
  mos.open(test_file2.c_str(), ios_base::out | ios_base::trunc);
  EXPECT_TRUE(mos.is_open());
  mos.write(data, sizeof(data));
  mos.close();
  EXPECT_FALSE(mos.is_open());

  string str;
  File::Path(test_file1).read_all(str);
  EXPECT_STREQ(data, str.c_str());
  str.clear();
  File::Path(test_file2).read_all(str);
  EXPECT_STREQ(data, str.c_str());
}

//--------------------------------------------------------------------------
// MultiOutStream open_back failure
TEST_F(MultiStreamTest, TestMultiOutStreamOpenBackFailure)
{
  File::MultiOutStream mos;
  EXPECT_FALSE(mos.open_back("/nonexistent/dir/file.txt",
                              ios_base::out | ios_base::trunc));
}

//--------------------------------------------------------------------------
// BufferedMultiOutStream set_buffer_size, is_open, tellp
TEST_F(BufferedMultiStreamTest, TestBufferedMultiStreamSetBufferSize)
{
  const string test_file(test_dir + "/bmos-resize");
  const char data[] = "buffered-multi";

  File::BufferedMultiOutStream mos{4};
  EXPECT_FALSE(mos.is_open());
  mos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  EXPECT_TRUE(mos.is_open());

  mos.set_buffer_size(8);
  EXPECT_EQ(8u, mos.get_buffer_size());

  mos.write(data, sizeof(data));
  EXPECT_EQ(sizeof(data), mos.tellp());
  mos.close();

  string str;
  File::Path(test_file).read_all(str);
  ASSERT_STREQ(data, str.c_str());
}

//--------------------------------------------------------------------------
// BufferedMultiOutStream open_back failure
TEST_F(BufferedMultiStreamTest, TestBufferedMultiStreamOpenBackFailure)
{
  File::BufferedMultiOutStream mos{4};
  EXPECT_FALSE(mos.open_back("/nonexistent/dir/file.txt",
                              ios_base::out | ios_base::trunc));
}

//--------------------------------------------------------------------------
// MultiOutStream single char write (triggers overflow)
TEST_F(MultiStreamTest, TestMultiOutStreamOverflow)
{
  const string test_file(test_dir + "/overflow");

  File::MultiOutStream mos;
  mos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  mos.put('X');
  mos.close();

  string str;
  File::Path(test_file).read_all(str);
  EXPECT_EQ("X", str);
}

//--------------------------------------------------------------------------
// BufferedMultiOutStream single char write (triggers overflow)
TEST_F(BufferedMultiStreamTest, TestBufferedMultiStreamOverflow)
{
  const string test_file(test_dir + "/bmos-overflow");

  File::BufferedMultiOutStream mos{4};
  mos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  mos.put('Y');
  mos.close();

  string str;
  File::Path(test_file).read_all(str);
  EXPECT_EQ("Y", str);
}

//--------------------------------------------------------------------------
// BufferedOutFileBuf resize to zero and resize smaller
TEST_F(StreamTest, TestBufferedOutFileBufResizeToZero)
{
  const string test_file(test_dir + "/resize-zero");
  const char data[] = "data";

  File::BufferedOutStream bos;
  bos.set_buffer_size(8);
  EXPECT_EQ(8u, bos.get_buffer_size());
  bos.set_buffer_size(0);
  EXPECT_EQ(0u, bos.get_buffer_size());

  bos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  bos.write(data, sizeof(data));
  bos.close();

  string str;
  File::Path(test_file).read_all(str);
  ASSERT_STREQ(data, str.c_str());
}

TEST_F(StreamTest, TestBufferedOutFileBufResizeSmaller)
{
  const string test_file(test_dir + "/resize-smaller");
  const char data[] = "data";

  File::BufferedOutStream bos;
  bos.set_buffer_size(16);
  EXPECT_EQ(16u, bos.get_buffer_size());
  bos.set_buffer_size(4);
  EXPECT_EQ(4u, bos.get_buffer_size());

  bos.open(test_file.c_str(), ios_base::out | ios_base::trunc);
  bos.write(data, sizeof(data));
  bos.close();

  string str;
  File::Path(test_file).read_all(str);
  ASSERT_STREQ(data, str.c_str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
