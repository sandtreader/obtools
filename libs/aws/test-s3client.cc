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
#include <iomanip>

// Uncomment and define with bucket credentials for testing
// DO NOT CHECK REAL KEYS INTO PUBLIC REPO - AWS will rightly disable them
// pretty much instantly
// #define KEY_ID "MY-ACCESS-KEY"
// #define SECRET_KEY "MY-SECRET-KEY-BASE64"
// #define BUCKETS_ROOT "my.bucket.domain"

namespace {

using namespace std;
using namespace ObTools;

// Switch between bucket name in path or in host name
bool use_virtual_hosts = false;

// All tests disabled until KEY_ID, SECRET_KEY, BUCKETS_ROOT defined above
#ifdef KEY_ID

class S3ClientTest: public ::testing::Test
{
  static constexpr auto test_access_key_id = KEY_ID;
  static constexpr auto test_secret_key = SECRET_KEY;
  static constexpr auto test_region = "us-east-1";

protected:
  static constexpr auto test_bucket = "test." BUCKETS_ROOT;
  static constexpr auto temp_bucket = "temp." BUCKETS_ROOT;

  AWS::S3Client *s3;

  virtual void SetUp()
  {
    s3 = new AWS::S3Client(test_access_key_id, test_secret_key, test_region);
    s3->enable_persistence();
    if (use_virtual_hosts) s3->enable_virtual_hosts();
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

TEST_F(S3ClientTest, TestAuthListAllMyBucketsTwiceForPersistence)
{
  Web::URL url("http://s3.amazonaws.com/");
  Web::HTTPMessage request1("GET", url);
  Web::HTTPMessage response1;
  ASSERT_TRUE(s3->do_request(request1, response1));
  ASSERT_EQ(200, response1.code);

  Web::HTTPMessage request2("GET", url);
  Web::HTTPMessage response2;
  ASSERT_TRUE(s3->do_request(request2, response2));
  ASSERT_EQ(200, response2.code);
}

TEST_F(S3ClientTest, TestAuthListABucket)
{
  Web::URL url("http://test." BUCKETS_ROOT ".s3.amazonaws.com/?delimiter=/");
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
  string url = s3->get_url("test." BUCKETS_ROOT).str();
  if (use_virtual_hosts)
    ASSERT_EQ("http://test." BUCKETS_ROOT ".s3.amazonaws.com/", url);
  else
    ASSERT_EQ("http://s3.amazonaws.com/test." BUCKETS_ROOT "/", url);
}

TEST_F(S3ClientTest, TestGetBucketURLForSpecificBucketAndObject)
{
  string url = s3->get_url("test." BUCKETS_ROOT, "foo/bar").str();
  if (use_virtual_hosts)
    ASSERT_EQ("http://test." BUCKETS_ROOT ".s3.amazonaws.com/foo/bar", url);
  else
    ASSERT_EQ("http://s3.amazonaws.com/test." BUCKETS_ROOT "/foo/bar", url);
}

TEST_F(S3ClientTest, TestListAllMyBuckets)
{
  set<string> buckets;
  ASSERT_TRUE(s3->list_all_my_buckets(buckets));
  ASSERT_LE(1, buckets.size());
  ASSERT_TRUE(buckets.find(test_bucket) != buckets.end());
}

TEST_F(S3ClientTest, TestListBucket)
{
  set<string> objects;
  ASSERT_TRUE(s3->list_bucket(test_bucket, objects));
  ASSERT_LE(1, objects.size());
  ASSERT_TRUE(objects.find("photos/paul.jpg") != objects.end());
}

TEST_F(S3ClientTest, TestListBucketWithPrefix)
{
  set<string> objects;
  ASSERT_TRUE(s3->list_bucket(test_bucket, objects, "photos/"));
  ASSERT_EQ(1, objects.size());
  ASSERT_TRUE(objects.find("photos/paul.jpg") != objects.end());
}

TEST_F(S3ClientTest, TestCreateAndDeleteBucketInDefaultRegion)
{
  EXPECT_TRUE(s3->create_bucket(temp_bucket));
  set<string> objects;
  EXPECT_TRUE(s3->list_bucket(temp_bucket, objects));
  EXPECT_EQ(0, objects.size());
  ASSERT_TRUE(s3->delete_bucket(temp_bucket));
}

TEST_F(S3ClientTest, TestCreateAndDeletePublicBucketInDefaultRegion)
{
  EXPECT_TRUE(s3->create_bucket(temp_bucket,
                                "public-read"));
  set<string> objects;
  EXPECT_TRUE(s3->list_bucket(temp_bucket, objects));
  EXPECT_EQ(0, objects.size());
  ASSERT_TRUE(s3->delete_bucket(temp_bucket));
}

TEST_F(S3ClientTest, TestCreateAndDeleteBucketInEURegion)
{
  // Doesn't work at all without virtual hosts because redirect always
  // points to a virtual host name
  if (!use_virtual_hosts) return;

  // Note expect so if creation fails because it exists it can still be deleted
  EXPECT_TRUE(s3->create_bucket("temp-eu." BUCKETS_ROOT, "",
                                "eu-west-1"));

  // Now we need to move to EU region for the rest
  s3->set_region("eu-west-1");

  set<string> objects;
  EXPECT_TRUE(s3->list_bucket("temp-eu." BUCKETS_ROOT, objects));
  EXPECT_EQ(0, objects.size());
  ASSERT_TRUE(s3->delete_bucket("temp-eu." BUCKETS_ROOT));
}

TEST_F(S3ClientTest, TestCreateGetAndDeleteObject)
{
  string data = "Mary had a little lamb";
  string key = "mary.txt";
  EXPECT_TRUE(s3->create_object(test_bucket, key, data));

  string readback;
  EXPECT_TRUE(s3->get_object(test_bucket, key,
                             readback));
  EXPECT_EQ(data, readback);

  EXPECT_TRUE(s3->delete_object(test_bucket, key));
}

TEST_F(S3ClientTest, TestPublicObjectsAreWorldReadable)
{
  string data = "Mary had a little lamb";
  string key = "mary.txt";
  EXPECT_TRUE(s3->create_object(test_bucket, key, data,
                                "public-read"));

  // Check readability with an ordinary HTTP client
  Web::URL url("http://test." BUCKETS_ROOT ".s3.amazonaws.com/mary.txt");
  Web::HTTPClient http(url);
  string readback;
  EXPECT_EQ(200, http.get(url, readback));
  EXPECT_EQ(data, readback);

  EXPECT_TRUE(s3->delete_object(test_bucket, key));
}

TEST_F(S3ClientTest, TestCreateAndList1500Objects)
{
  const string bucket_name("test-1500." BUCKETS_ROOT);
  // Check if bucket exists - if not, create it and add objects
  set<string> buckets;
  ASSERT_TRUE(s3->list_all_my_buckets(buckets));
  if (buckets.find(bucket_name) == buckets.end())
  {
    EXPECT_TRUE(s3->create_bucket(bucket_name));

    string data = "Mary had a little lamb";
    for(int i=0; i<1500; i++)
    {
      ostringstream oss;
      oss << "mary." << setfill('0') << setw(4) << i << ".txt";
      EXPECT_TRUE(s3->create_object(bucket_name, oss.str(), data));
    }
  }

  set<string> objects;
  ASSERT_TRUE(s3->list_bucket(bucket_name, objects));
  ASSERT_EQ(1500, objects.size());
  int i=0;
  for(const auto& key: objects)
  {
    ostringstream oss;
    oss << "mary." << setfill('0') << setw(4) << i++ << ".txt";
    EXPECT_EQ(oss.str(), key);
  }

  // Delete manually from console - it's too big to recreate each time!
}

TEST_F(S3ClientTest, TestDeleteMultipleObjects)
{
  // Create 10 temp objects
  string data = "Hello, world!";
  set<string> keys;
  for(int i=0; i<10; i++)
  {
    ostringstream oss;
    oss << "temp/hw." << i << ".txt";
    keys.insert(oss.str());
    EXPECT_TRUE(s3->create_object(test_bucket,
                                  oss.str(), data));
  }

  // Try to delete them again, forcing max_keys to 6 to test chunking
  EXPECT_TRUE(s3->delete_multiple_objects(test_bucket,keys,6));

  // Check they are gone
  set<string> dkeys;
  EXPECT_TRUE(s3->list_bucket(test_bucket, dkeys, "temp/"));
  EXPECT_EQ(0, dkeys.size());
}

TEST_F(S3ClientTest, TestDeleteObjectsWithPrefix)
{
  // Create some temp objects
  string data = "Hello, world!";
  for(int i=0; i<3; i++)
  {
    ostringstream oss;
    oss << "temp/hw." << i << ".txt";
    EXPECT_TRUE(s3->create_object(test_bucket,
                                  oss.str(), data));
  }

  EXPECT_TRUE(s3->delete_objects_with_prefix(test_bucket,
                                             "temp/"));

  // Check they are gone
  set<string> keys;
  EXPECT_TRUE(s3->list_bucket(test_bucket, keys, "temp/"));
  EXPECT_EQ(0, keys.size());
}

TEST_F(S3ClientTest, TestEmptyBucket)
{
  EXPECT_TRUE(s3->create_bucket(temp_bucket));
  // Create some temp objects
  string data = "Hello, world!";
  for(int i=0; i<3; i++)
  {
    ostringstream oss;
    oss << "temp/hw." << i << ".txt";
    EXPECT_TRUE(s3->create_object(temp_bucket, oss.str(), data));
  }

  EXPECT_TRUE(s3->empty_bucket(temp_bucket));

  set<string> objects;
  EXPECT_TRUE(s3->list_bucket(temp_bucket, objects));
  EXPECT_EQ(0, objects.size());
  ASSERT_TRUE(s3->delete_bucket(temp_bucket));
}

#endif // KEY_ID defined

} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_err = new Log::StreamChannel{&cerr};
    Log::logger.connect(chan_err);
  }

  ::testing::InitGoogleTest(&argc, argv);

  // Run without virtual hosts first
  cerr << "====== Tests without virtual host URLs =====\n";
  int rc = RUN_ALL_TESTS();
  if (rc) return rc;

  // Then with
  cerr << "====== Tests with virtual host URLs =====\n";
  use_virtual_hosts = true;
  return RUN_ALL_TESTS();
}
