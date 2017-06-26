//==========================================================================
// ObTools::AWS: test-auth.cc
//
// Tests for AWS authentication
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

// Access keys from AWS documentation
// http://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-header-based-auth.html
// (accessed 22.06.2017)
static constexpr auto example_access_key_id = "AKIAIOSFODNN7EXAMPLE";
static constexpr auto example_secret_key =
  "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY";

// Example from
// http://docs.aws.amazon.com/general/latest/gr/signature-v4-examples.html
TEST(AuthTest, TestCreateSecretHex)
{
  string key = string("AWS4")+example_secret_key;
  string key_hex = Text::btox(key);
  ASSERT_EQ("41575334774a616c725855746e46454d492f4b374d44454e472b62507852666943594558414d504c454b4559", key_hex);
}

// Example from
// http://docs.aws.amazon.com/general/latest/gr/signature-v4-examples.html
TEST(AuthTest, TestDerivingSigningKey1)
{
  AWS::Authenticator auth(example_access_key_id, example_secret_key);
  Time::Stamp date("20120215T000000Z");
  const auto key = auth.get_signing_key(date, "us-east-1", "iam");
  const auto hex_key = Text::btox(key);
  ASSERT_EQ("f4780e2d9f65fa895f9c67b32ce1baf0b0d8a43505a000a1a9e090d414db404d",
            hex_key);
}

// Example from
// http://docs.aws.amazon.com/general/latest/gr/sigv4-calculate-signature.html
TEST(AuthTest, TestDerivingSigningKey2)
{
  AWS::Authenticator auth(example_access_key_id, example_secret_key);
  Time::Stamp date("20150830T000000Z");
  const auto key = auth.get_signing_key(date, "us-east-1", "iam");
  const auto hex_key = Text::btox(key);
  ASSERT_EQ("c4afb1cc5771d871763a393e44b703571b55cc28424d1a5e86da6ed3c154a4b9",
            hex_key);
}

// Example from
// http://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-header-based-auth.html
TEST(AuthTest, TestIndividualRequestSignatureOperations)
{
  // Note!  In this example the key one character different!
  string secret_key = "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY";
  AWS::Authenticator auth(example_access_key_id, secret_key);
  Misc::PropertyList query, headers;
  headers.add("Host", "examplebucket.s3.amazonaws.com");
  headers.add("Range", "bytes=0-9");
  Time::Stamp date("20130524T000000Z");
  string payload; // empty

  // ------ Create canonical request -------
  const auto req = auth.create_canonical_request("GET", "/test.txt", date,
                                                 query, headers, payload);
  ASSERT_EQ(R"(GET
/test.txt

host:examplebucket.s3.amazonaws.com
range:bytes=0-9
x-amz-content-sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
x-amz-date:20130524T000000Z

host;range;x-amz-content-sha256;x-amz-date
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855)", req);

  // ------- Create string to sign --------
  const auto sts = auth.get_string_to_sign(req, date,
                                       "20130524/us-east-1/s3/aws4_request");
  ASSERT_EQ(R"(AWS4-HMAC-SHA256
20130524T000000Z
20130524/us-east-1/s3/aws4_request
7344ae5b7ee6c3e7e6b0fe0640412a37625d1fbfff95c48bbb2dc43964946972)", sts);

  // ------- Signing key ---------
  const auto key = auth.get_signing_key(date, "us-east-1", "s3");
  // ! Example doesn't say what this key should be

  // ------- Signature ---------
  const auto sig = auth.sign(key, sts);
  ASSERT_EQ("f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41",
            sig);
}

TEST(AuthTest, TestCombinedRequestSignature)
{
  // Note!  In this example the key one character different!
  string secret_key = "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY";
  AWS::Authenticator auth(example_access_key_id, secret_key);
  Misc::PropertyList query, headers;
  headers.add("Host", "examplebucket.s3.amazonaws.com");
  headers.add("Range", "bytes=0-9");
  Time::Stamp date("20130524T000000Z");
  string payload; // empty

  const auto sig = auth.get_signature("GET", "/test.txt", date,
                                      query, headers, payload,
                                      "us-east-1", "s3");
  ASSERT_EQ("f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41",
            sig);
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
