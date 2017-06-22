//==========================================================================
// ObTools::AWS: test-s3client.cc
//
// Tests for AWS S3 client implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-aws.h"
#include "ot-log.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

class S3ClientTest: public ::testing::Test
{
  static constexpr auto test_access_key_id = "AKIAJL4EKQVW7RRLB23Q";
  static constexpr auto test_secret_key =
    "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";

protected:
  AWS::S3Client *s3;

  virtual void SetUp()
  {
    s3 = new AWS::S3Client(test_access_key_id, test_secret_key);
  }

  virtual void TearDown()
  {
    delete s3;
  }

public:
  S3ClientTest()
  {}
};

TEST_F(S3ClientTest, TestBasicConnection)
{
}


} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_err = new Log::StreamChannel{&cerr};
    Log::logger.connect(chan_err);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
