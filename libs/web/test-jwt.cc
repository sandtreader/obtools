//==========================================================================
// ObTools::Web: test-jwt.cc
//
// Test harness for JSON Web Token handling
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include "ot-web.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(JWTTest, TestBasicParsing)
{
  // From jwt.io debugger
  string encoded("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c");

  Web::JWT jwt(encoded);
  ASSERT_FALSE(!jwt);

  EXPECT_EQ("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9", jwt.header_b64);
  EXPECT_EQ("eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ", jwt.payload_b64);
  EXPECT_EQ("SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c", jwt.signature_b64);

  EXPECT_EQ("HS256", jwt.header["alg"].as_str());
  EXPECT_EQ("JWT", jwt.header["typ"].as_str());

  EXPECT_EQ("1234567890", jwt.payload["sub"].as_str());
  EXPECT_EQ("John Doe", jwt.payload["name"].as_str());
  EXPECT_EQ(1516239022, jwt.payload["iat"].as_int());
}

TEST(JWTTest, TestVerificationJWT)
{
  // From jwt.io debugger
  string encoded("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c");

  Web::JWT jwt(encoded);
  ASSERT_TRUE(jwt.verify("your-256-bit-secret"));
}

TEST(JWTTest, TestVerificationBadJWT)
{
  // From jwt.io debugger, last character changed
  string encoded("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5b");

  Web::JWT jwt(encoded);
  ASSERT_FALSE(jwt.verify("your-256-bit-secret"));
}

TEST(JWTTest, TestVerificationRFC7519)
{
  // From RFC 7519 example
  string encoded("eyJ0eXAiOiJKV1QiLA0KICJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFtcGxlLmNvbS9pc19yb290Ijp0cnVlfQ.dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk");

  // Buried in RFC7515 Appendix A
  string key_enc("AyM1SysPpbyDfgZld3umj1qzKObwVMkoqQ-EstJQLr_T-1qS0gZH75aKtMN3Yj0iPS4hcgUuTwjAzZr1Z9CAow");

  Web::JWT jwt(encoded);
  ASSERT_FALSE(!jwt);

  Text::Base64URL base64;
  string key;
  ASSERT_TRUE(base64.decode(key_enc, key));
  ASSERT_TRUE(jwt.verify(key));
}

TEST(JWTTest, TestJSONConstruction)
{
  JSON::Value payload(JSON::Value::OBJECT);
  payload.set("claim", "CLAIM");
  Web::JWT jwt(payload);
  ASSERT_TRUE(!jwt);  // Because not signed
  EXPECT_EQ("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9", jwt.header_b64);
  EXPECT_EQ("eyJjbGFpbSI6IkNMQUlNIn0", jwt.payload_b64);
}

TEST(JWTTest, TestJSONSignStrAndReverify)
{
  JSON::Value payload(JSON::Value::OBJECT);
  payload.set("claim", "CLAIM");
  Web::JWT jwt(payload);
  jwt.sign("secret");
  EXPECT_EQ("P-BD4ngX0SQm0b4s8SFjlwEXc2fABcrYKSfXXq7uNLw", jwt.signature_b64);
  EXPECT_EQ("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJjbGFpbSI6IkNMQUlNIn0.P-BD4ngX0SQm0b4s8SFjlwEXc2fABcrYKSfXXq7uNLw", jwt.str());
  EXPECT_TRUE(jwt.verify("secret"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    auto level_out = new Log::LevelFilter{chan_out, Log::Level::detail};
    Log::logger.connect(level_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
