//==========================================================================
// ObTools::XML: test-config.cc
//
// Test harness for ObTools XML configuration
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
#include "ot-file.h"
#include "ot-log.h"
#include <iostream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

const auto test_dir = string{"/tmp/ot-config"};
const auto toplevel_config_file = test_dir + "/toplevel.xml";

const auto toplevel_config = R"(
<!--
  Test config
-->

<toplevel>
  <element attr="foo"/>
</toplevel>
)";

class ConfigurationTest: public ::testing::Test
{
protected:

  virtual void SetUp()
  {
    // Create state dirs
    File::Directory dir(test_dir);
    dir.ensure(true);
    File::Path toplevel_path(toplevel_config_file);
    toplevel_path.write_all(toplevel_config);
  }

  virtual void TearDown()
  {
    File::Directory state(test_dir);
    state.erase();
  }

public:
  ConfigurationTest() {}
};

TEST_F(ConfigurationTest, TestReadConfig)
{
  Log::Streams log;
  XML::Configuration config(toplevel_config_file, log.error);
  ASSERT_TRUE(config.read("toplevel"));
  EXPECT_EQ("foo", config["element/@attr"]);
}


} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

