//==========================================================================
// ObTools::SOAP: test-message-interface.cc
//
// Tests for the SOAP message interface's servers - that one which can't
// listen (e.g. its port is in use) says so, rather than silently serving
// nothing
//
// Copyright (c) 2026 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ObTools;

// Log output, for checking what's logged - the logger owns its channels
// and can't disconnect them, so connect once and keep it
ostringstream& log_out()
{
  static ostringstream *out = nullptr;
  if (!out)
  {
    out = new ostringstream;
    Log::logger.connect(new Log::StreamChannel{out});
  }
  return *out;
}

struct Context {};

// <soap> config with an HTTP server on the given port (0 for none)
XML::Element soap_config(int port)
{
  XML::Element soap("soap");
  if (port)
    soap.add("server").set_attr_int("port", port);
  return soap;
}

class MessageInterfaceTest: public ::testing::Test
{
protected:
  Context context;
  Init::Registry<Message::Handler<Context> > registry;
  Message::Broker<Context> broker{registry};

  // Connect the log (first time) and start each test with it empty
  void SetUp() override { log_out().str(""); }
};

//--------------------------------------------------------------------------
// With its port free, the server is listening
TEST_F(MessageInterfaceTest, TestListeningOnFreePort)
{
  SOAP::MessageInterface<Context> soap(context, soap_config(11121), broker,
                                       "test");
  EXPECT_TRUE(soap.is_listening());
}

//--------------------------------------------------------------------------
// With no server configured, there is nothing that failed to listen
TEST_F(MessageInterfaceTest, TestNoServerConfiguredIsListening)
{
  SOAP::MessageInterface<Context> soap(context, soap_config(0), broker,
                                       "test");
  EXPECT_TRUE(soap.is_listening());
}

//--------------------------------------------------------------------------
// With its port in use, it isn't listening, and logs an error naming the
// address and why
TEST_F(MessageInterfaceTest, TestPortInUseNotListeningAndLogged)
{
  SOAP::MessageInterface<Context> first(context, soap_config(11122), broker,
                                        "test");
  ASSERT_TRUE(first.is_listening());

  SOAP::MessageInterface<Context> second(context, soap_config(11122), broker,
                                         "test");
  EXPECT_FALSE(second.is_listening());

  const auto log = log_out().str();
  EXPECT_NE(string::npos, log.find("Can't listen")) << log;
  EXPECT_NE(string::npos, log.find(":11122")) << log;
  EXPECT_NE(string::npos, log.find(strerror(EADDRINUSE))) << log;
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
