//==========================================================================
// ObTools::Net: test-server.cc
//
// Test harness for TCP server functions
//
// Copyright (c) 2003-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-net.h"
#include <stdlib.h>
#include <cerrno>

using namespace std;

//--------------------------------------------------------------------------
// Test server class
class TestServer: public ObTools::Net::TCPServer
{
public:
  TestServer(int port): ObTools::Net::TCPServer(port) {}

  void process(ObTools::Net::TCPSocket& s,
               ObTools::Net::EndPoint client);
};

void TestServer::process(ObTools::Net::TCPSocket& s,
                         ObTools::Net::EndPoint client)
{
  cerr << "Got connection from " << client
       << " (" << s.get_mac(client.host) << ")" << endl;

  try
  {
    // Just output and reflect what we get
    string buf;
    while (s >> buf)
    {
      cout << buf;
      s << "<< " << buf << "\n";
    }

    cerr << "Connection from " << client << " ended\n";
  }
  catch (const ObTools::Net::SocketError& se)
  {
    cerr << se << endl;
  }
}


TEST(TCPServerTests, TestServerExits)
{
  int port = 11111;

  cout << "Starting server on port " << port << endl;
  TestServer server(port);
  ObTools::Net::TCPServerThread server_thread(server);

  cout << "Started\n";
  this_thread::sleep_for(chrono::seconds{1});

  cout << "Shutting down\n";
  server.shutdown();

  cout << "Exiting\n";
}

//--------------------------------------------------------------------------
// A server that is listening has no start error
TEST(TCPServerTests, TestListeningServerHasNoStartError)
{
  TestServer server(11112);
  EXPECT_EQ(0, server.get_start_error());
  EXPECT_FALSE(!server);
  server.shutdown();
}

//--------------------------------------------------------------------------
// A server whose port is already in use says why it isn't listening
TEST(TCPServerTests, TestPortInUseGivesStartError)
{
  TestServer first(11113);
  ASSERT_EQ(0, first.get_start_error());

  TestServer second(11113);
  EXPECT_EQ(EADDRINUSE, second.get_start_error());
  EXPECT_TRUE(!second);

  second.shutdown();
  first.shutdown();
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



