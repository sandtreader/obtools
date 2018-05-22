//==========================================================================
// ObTools::Web: test-http-client.cc
//
// GTest test harness for HTTP client
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(HTTPClientTest, TestHTTPSWithNoSSLFailsRequests)
{
  Web::URL url("https://example.com");
  Web::HTTPClient http(url);  // Note no ssl_ctx
  string body;
  EXPECT_EQ(-1, http.get(url, body));
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
