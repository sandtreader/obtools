//==========================================================================
// ObTools::Web: test-compression.cc
//
// GTest harness for gzip compression of HTTP responses
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-web.h"
#include <gtest/gtest.h>
#include <zlib.h>

namespace {

using namespace std;
using namespace ObTools;

// Decompress gzip data with zlib directly, independent of the code under
// test - returns false if it isn't valid gzip
bool gunzip(const string& in, string& out)
{
  z_stream zs{};
  if (inflateInit2(&zs, 15 + 16) != Z_OK) return false;
  zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.data()));
  zs.avail_in = in.size();
  out.clear();
  char buf[16384];
  int ret;
  do
  {
    zs.next_out = reinterpret_cast<Bytef *>(buf);
    zs.avail_out = sizeof(buf);
    ret = inflate(&zs, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) { inflateEnd(&zs); return false; }
    out.append(buf, sizeof(buf) - zs.avail_out);
  } while (ret != Z_STREAM_END);
  inflateEnd(&zs);
  return zs.avail_in == 0;
}

// Highly compressible text of about the given size
string text_of_size(size_t size)
{
  string s;
  for(auto i=0; s.size() < size; i++)
    s += "<ps:record type=\"epg-1m\">{\"avg\":{\"epg\":{\"up\":1}}}</ps:record>"
         + to_string(i % 10);
  return s;
}

//==========================================================================
// GZip::compress

TEST(GZipTest, TestCompressRoundTrip)
{
  const auto in = text_of_size(100000);
  string out;
  ASSERT_TRUE(Web::GZip::compress(in, out));
  ASSERT_GE(out.size(), 2u);
  EXPECT_EQ('\x1f', out[0]);  // gzip magic
  EXPECT_EQ('\x8b', out[1]);
  EXPECT_LT(out.size(), in.size()/10);

  string back;
  ASSERT_TRUE(gunzip(out, back));
  EXPECT_EQ(in, back);
}

TEST(GZipTest, TestCompressLevels)
{
  const auto in = text_of_size(100000);
  string fast, best;
  ASSERT_TRUE(Web::GZip::compress(in, fast, 1));
  ASSERT_TRUE(Web::GZip::compress(in, best, 9));
  string back;
  ASSERT_TRUE(gunzip(fast, back));
  EXPECT_EQ(in, back);
  ASSERT_TRUE(gunzip(best, back));
  EXPECT_EQ(in, back);
  EXPECT_LE(best.size(), fast.size());
}

TEST(GZipTest, TestCompressEmpty)
{
  string out;
  ASSERT_TRUE(Web::GZip::compress("", out));
  string back = "x";
  ASSERT_TRUE(gunzip(out, back));
  EXPECT_EQ("", back);
}

TEST(GZipTest, TestCompressBinary)
{
  string in;
  for(auto i=0; i<5000; i++) in += static_cast<char>(i * 7 % 256);
  string out, back;
  ASSERT_TRUE(Web::GZip::compress(in, out));
  ASSERT_TRUE(gunzip(out, back));
  EXPECT_EQ(in, back);
}

//==========================================================================
// HTTPMessage::accepts_encoding

bool accepts(const string& accept_encoding, const string& coding = "gzip")
{
  Web::HTTPMessage request("POST", Web::URL("/list-records"));
  if (!accept_encoding.empty())
    request.headers.put("Accept-Encoding", accept_encoding);
  return request.accepts_encoding(coding);
}

TEST(AcceptsEncodingTest, TestListed)
{
  EXPECT_TRUE(accepts("gzip"));
  EXPECT_TRUE(accepts("gzip, deflate, br"));
  EXPECT_TRUE(accepts("deflate, br, gzip"));
  EXPECT_TRUE(accepts("deflate,gzip"));
  EXPECT_TRUE(accepts("GZip"));        // Case insensitive
  EXPECT_TRUE(accepts("x-gzip"));      // Alias of gzip (RFC 9110)
}

TEST(AcceptsEncodingTest, TestNotListed)
{
  EXPECT_FALSE(accepts(""));           // No header
  EXPECT_FALSE(accepts("deflate, br"));
  EXPECT_FALSE(accepts("identity"));
  EXPECT_FALSE(accepts("gzipx"));
}

TEST(AcceptsEncodingTest, TestQValues)
{
  EXPECT_TRUE(accepts("gzip;q=0.5"));
  EXPECT_TRUE(accepts("gzip; q=1.0, deflate"));
  EXPECT_FALSE(accepts("gzip;q=0"));
  EXPECT_FALSE(accepts("gzip;q=0.0, deflate"));
  EXPECT_FALSE(accepts("gzip; q=0.000"));
  EXPECT_TRUE(accepts("gzip;q=0.001"));
}

TEST(AcceptsEncodingTest, TestWildcard)
{
  EXPECT_TRUE(accepts("*"));
  EXPECT_TRUE(accepts("br, *;q=0.1"));
  EXPECT_FALSE(accepts("*;q=0"));
  EXPECT_FALSE(accepts("identity, *;q=0"));
  // An explicit coding overrides the wildcard
  EXPECT_FALSE(accepts("*, gzip;q=0"));
  EXPECT_TRUE(accepts("gzip, *;q=0"));
}

TEST(AcceptsEncodingTest, TestReadFromRequest)
{
  // Headers read from the wire are lower-cased
  istringstream iss("POST /list-records HTTP/1.1\r\n"
                    "Host: core\r\n"
                    "Accept-Encoding: gzip, deflate, br\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n");
  Web::HTTPMessage request;
  ASSERT_TRUE(request.read(iss));
  EXPECT_TRUE(request.accepts_encoding("gzip"));
  EXPECT_FALSE(request.accepts_encoding("compress"));
}

TEST(AcceptsEncodingTest, TestOtherCoding)
{
  EXPECT_TRUE(accepts("gzip, br", "br"));
  EXPECT_FALSE(accepts("gzip", "br"));
}

//==========================================================================
// compress_response

struct CompressResponseTest: public ::testing::Test
{
  Web::HTTPMessage request{"POST", Web::URL("/list-records")};
  Web::HTTPMessage response{200, "OK"};
  string body = text_of_size(10000);

  void SetUp() override
  {
    request.headers.put("Accept-Encoding", "gzip, deflate, br");
    response.headers.put("Content-Type", "application/soap+xml");
    response.body = body;
  }

  bool compress(size_t min_size = 1024)
  { return Web::compress_response(request, response, min_size); }

  void expect_unchanged()
  {
    EXPECT_EQ(body, response.body);
    EXPECT_FALSE(response.headers.has("Content-Encoding"));
  }
};

TEST_F(CompressResponseTest, TestCompressesWhenAccepted)
{
  ASSERT_TRUE(compress());
  EXPECT_EQ("gzip", response.headers.get("Content-Encoding"));
  EXPECT_EQ("Accept-Encoding", response.headers.get("Vary"));
  EXPECT_LT(response.body.size(), body.size());
  string back;
  ASSERT_TRUE(gunzip(response.body, back));
  EXPECT_EQ(body, back);
}

TEST_F(CompressResponseTest, TestNotWhenNotAccepted)
{
  request.headers.remove("Accept-Encoding");
  EXPECT_FALSE(compress());
  expect_unchanged();

  request.headers.put("Accept-Encoding", "gzip;q=0");
  EXPECT_FALSE(compress());
  expect_unchanged();
}

TEST_F(CompressResponseTest, TestThreshold)
{
  // Below the minimum size - not worth it
  body = text_of_size(500);
  response.body = body;
  EXPECT_FALSE(compress(1024));
  expect_unchanged();

  // At or above it
  body = string(1024, 'x');
  response.body = body;
  EXPECT_TRUE(compress(1024));
  EXPECT_EQ("gzip", response.headers.get("Content-Encoding"));
}

TEST_F(CompressResponseTest, TestZeroThresholdDisables)
{
  EXPECT_FALSE(compress(0));
  expect_unchanged();
}

TEST_F(CompressResponseTest, TestNotForHead)
{
  request.method = "HEAD";
  EXPECT_FALSE(compress());
  expect_unchanged();
}

TEST_F(CompressResponseTest, TestNotIfAlreadyEncoded)
{
  response.headers.put("Content-Encoding", "br");
  EXPECT_FALSE(compress());
  EXPECT_EQ(body, response.body);
  EXPECT_EQ("br", response.headers.get("Content-Encoding"));
}

TEST_F(CompressResponseTest, TestNotWithExplicitLength)
{
  // Set by handlers sending more data progressively after the body
  response.headers.put("Content-Length", "1000000");
  EXPECT_FALSE(compress());
  expect_unchanged();

  response.headers.remove("Content-Length");
  response.headers.put("content-length", "1000000");
  EXPECT_FALSE(compress());
  expect_unchanged();

  response.headers.remove("content-length");
  response.headers.put("Transfer-Encoding", "chunked");
  EXPECT_FALSE(compress());
  expect_unchanged();
}

TEST_F(CompressResponseTest, TestLowerCaseHeaders)
{
  response.headers.remove("Content-Type");
  response.headers.put("content-type", "image/jpeg");
  EXPECT_FALSE(compress());
  expect_unchanged();

  response.headers.remove("content-type");
  response.headers.put("content-encoding", "br");
  EXPECT_FALSE(compress());
  EXPECT_EQ(body, response.body);
}

TEST_F(CompressResponseTest, TestNotForCompressedTypes)
{
  for(const auto type: { "image/jpeg", "image/png", "video/mp2t",
                         "audio/mpeg", "application/zip",
                         "application/gzip", "application/x-gzip" })
  {
    response.headers.replace("Content-Type", type);
    EXPECT_FALSE(compress()) << type;
    expect_unchanged();
  }

  // But compressible ones are
  response.headers.replace("Content-Type", "image/svg+xml");
  EXPECT_TRUE(compress());
}

TEST_F(CompressResponseTest, TestAddsToExistingVary)
{
  response.headers.put("Vary", "Origin");
  ASSERT_TRUE(compress());
  EXPECT_EQ("Origin, Accept-Encoding", response.headers.get("Vary"));
}

TEST_F(CompressResponseTest, TestWrittenWithCompressedLength)
{
  ASSERT_TRUE(compress());
  ostringstream oss;
  ASSERT_TRUE(response.write(oss));
  const auto s = oss.str();
  EXPECT_NE(string::npos,
            s.find("Content-Length: " + to_string(response.body.size())
                   + "\r\n"));
}

TEST_F(CompressResponseTest, TestLevel)
{
  ASSERT_TRUE(Web::compress_response(request, response, 1024, 1));
  string back;
  ASSERT_TRUE(gunzip(response.body, back));
  EXPECT_EQ(body, back);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
