//==========================================================================
// ObTools::SOAP: test-server-options.cc
//
// GTest harness for SOAP HTTP server options
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-soap.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

SOAP::ServerOptions read_options(const string& xml)
{
  XML::Parser parser;
  parser.read_from(xml);
  SOAP::ServerOptions options;
  options.read(parser.get_root());
  return options;
}

TEST(ServerOptionsTest, TestDefaults)
{
  const SOAP::ServerOptions options;
  EXPECT_EQ(5, options.backlog);
  EXPECT_EQ(1, options.min_spare);
  EXPECT_EQ(10, options.max_threads);
  EXPECT_EQ(90, options.timeout);
}

TEST(ServerOptionsTest, TestReadNone)
{
  const auto options = read_options("<server port='7080'/>");
  EXPECT_EQ(5, options.backlog);
  EXPECT_EQ(1, options.min_spare);
  EXPECT_EQ(10, options.max_threads);
  EXPECT_EQ(90, options.timeout);
}

TEST(ServerOptionsTest, TestReadAll)
{
  const auto options = read_options(
    "<server port='7080' backlog='20' min-spare='0' max-threads='50'"
    " timeout='30'/>");
  EXPECT_EQ(20, options.backlog);
  EXPECT_EQ(0, options.min_spare);
  EXPECT_EQ(50, options.max_threads);
  EXPECT_EQ(30, options.timeout);
}

TEST(ServerOptionsTest, TestBadValuesIgnored)
{
  const auto options = read_options(
    "<server backlog='0' min-spare='-1' max-threads='0' timeout='-5'/>");
  EXPECT_EQ(5, options.backlog);
  EXPECT_EQ(1, options.min_spare);
  EXPECT_EQ(10, options.max_threads);
  EXPECT_EQ(90, options.timeout);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
