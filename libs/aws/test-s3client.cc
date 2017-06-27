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
  static constexpr auto test_region = "us-east-1";

protected:
  AWS::S3Client *s3;

  virtual void SetUp()
  {
    s3 = new AWS::S3Client(test_access_key_id, test_secret_key, test_region);
  }

  virtual void TearDown()
  {
    delete s3;
  }

public:
  S3ClientTest()
  {}
};

TEST_F(S3ClientTest, TestAuthListAllMyBuckets)
{
  Web::URL url("http://s3.amazonaws.com/");
  Web::HTTPMessage request("GET", url);
  Web::HTTPMessage response;
  ASSERT_TRUE(s3->do_request(request, response));
  ASSERT_EQ(200, response.code);
}

TEST_F(S3ClientTest, TestAuthListABucket)
{
  Web::URL url("http://test.buckets.packetship.com.s3.amazonaws.com/?delimiter=/");
  Web::HTTPMessage request("GET", url);
  Web::HTTPMessage response;
  ASSERT_TRUE(s3->do_request(request, response));
  ASSERT_EQ(200, response.code);
}

TEST_F(S3ClientTest, TestXMLListAllMyBuckets)
{
  Web::URL url("http://s3.amazonaws.com/");
  XML::Element response;
  ASSERT_TRUE(s3->do_request(url, response));
  ASSERT_EQ("ListAllMyBucketsResult", response.name);
}

TEST_F(S3ClientTest, TestXMLListNonexistentBucket)
{
  Web::URL url("http://does.not.exist.s3.amazonaws.com/");
  XML::Element response;
  ASSERT_FALSE(s3->do_request(url, response));
}

TEST_F(S3ClientTest, TestGetBucketURLForAllBuckets)
{
  ASSERT_EQ("http://s3.amazonaws.com/", s3->get_url().str());
}

TEST_F(S3ClientTest, TestGetBucketURLForSpecificBucket)
{
  ASSERT_EQ("http://test.packetship.com.s3.amazonaws.com/",
            s3->get_url("test.packetship.com").str());
}

TEST_F(S3ClientTest, TestGetBucketURLForSpecificBucketAndObject)
{
  ASSERT_EQ("http://test.packetship.com.s3.amazonaws.com/foo/bar",
            s3->get_url("test.packetship.com", "foo/bar").str());
}

TEST_F(S3ClientTest, TestListAllMyBuckets)
{
  set<string> buckets;
  ASSERT_TRUE(s3->list_all_my_buckets(buckets));
  ASSERT_LE(1, buckets.size());
  ASSERT_TRUE(buckets.find("test.buckets.packetship.com") != buckets.end());
}

TEST_F(S3ClientTest, TestListBucket)
{
  set<string> objects;
  ASSERT_TRUE(s3->list_bucket("test.buckets.packetship.com", objects));
  ASSERT_LE(1, objects.size());
  ASSERT_TRUE(objects.find("photos/paul.jpg") != objects.end());
}

TEST_F(S3ClientTest, TestCreateAndDeleteBucketInDefaultRegion)
{
  EXPECT_TRUE(s3->create_bucket("test-temp.buckets.packetship.com"));
  set<string> objects;
  EXPECT_TRUE(s3->list_bucket("test-temp.buckets.packetship.com", objects));
  EXPECT_EQ(0, objects.size());
  ASSERT_TRUE(s3->delete_bucket("test-temp.buckets.packetship.com"));
}

TEST_F(S3ClientTest, TestCreateAndDeleteBucketInEURegion)
{
  // Note expect so if creation fails because it exists it can still be deleted
  EXPECT_TRUE(s3->create_bucket("test-temp-eu.buckets.packetship.com",
                                "eu-west-1"));

  // Now we need to move to EU region for the rest
  s3->set_region("eu-west-1");

  set<string> objects;
  EXPECT_TRUE(s3->list_bucket("test-temp-eu.buckets.packetship.com", objects));
  EXPECT_EQ(0, objects.size());
  ASSERT_TRUE(s3->delete_bucket("test-temp-eu.buckets.packetship.com"));
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
