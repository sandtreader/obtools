//==========================================================================
// ObTools::File: test-stream.cc
//
// Test harness for file path library
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
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
    bos.write(data, sizeof(data));
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

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
