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
#include <thread>

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

TEST(HTTPClientTest, TestCustomContentTypePOST)
{
  Net::TCPSingleServer server(0);
  int port = server.local().port;

  string seen_content_type;
  string seen_body;

  thread server_thread([&]() {
    Net::TCPSocket *sock = server.wait(5);
    if (!sock) return;

    Net::TCPStream stream(*sock);
    Web::HTTPMessage request;
    if (!request.read(stream, true))
    {
      delete sock;
      return;
    }

    seen_content_type = request.headers.get("content-type");
    seen_body = request.body;

    Web::HTTPMessage response(200, "OK");
    response.body = "{\"status\":\"ok\"}";
    response.headers.put("content-type", "application/json");
    response.write(stream);
    stream.flush();

    delete sock;
  });

  Web::URL url("http://127.0.0.1:" + to_string(port) + "/echo");
  Web::HTTPClient client(url);
  string response_body;

  EXPECT_EQ(200, client.post(url, "{\"hello\":\"world\"}", response_body,
                            "application/json"));
  EXPECT_EQ("{\"status\":\"ok\"}", response_body);

  server_thread.join();

  EXPECT_EQ("application/json", seen_content_type);
  EXPECT_EQ("{\"hello\":\"world\"}", seen_body);
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
