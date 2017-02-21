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
const auto sub1_config_file = test_dir + "/sub1.xml";
const auto sub2_config_file = test_dir + "/sub2.xml";

const auto toplevel_config = R"(
<toplevel>
  <element attr="foo"/>
</toplevel>
)";

const auto toplevel_config_with_include = R"(
<toplevel>
  <include file="sub1.xml"/>
  <element attr="foo"/>
</toplevel>
)";

const auto toplevel_config_with_pattern = R"(
<toplevel>
  <include file="sub*.xml"/>
  <element attr="foo"/>
</toplevel>
)";

const auto sub1_config = R"(
<toplevel>
  <element attr="bar"/>
</toplevel>
)";

const auto sub2_config = R"(
<toplevel>
  <element id="different" attr="NOTME"/>
  <element2 attr2="bar2"/>
</toplevel>
)";

class ConfigurationTest: public ::testing::Test
{
protected:

  virtual void SetUp()
  {
    File::Directory dir(test_dir);
    dir.ensure(true);
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
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config);

  Log::Streams log;
  XML::Configuration config(toplevel_config_file, log.error);
  ASSERT_TRUE(config.read("toplevel"));
  EXPECT_EQ("foo", config["element/@attr"]);
}

TEST_F(ConfigurationTest, TestIncludeSub1Config)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config_with_include);
  File::Path sub1_path(sub1_config_file);
  sub1_path.write_all(sub1_config);

  Log::Streams log;
  XML::Configuration config(toplevel_config_file, log.error);
  ASSERT_TRUE(config.read("toplevel"));

  config.process_includes();
  EXPECT_EQ("bar", config["element/@attr"]);
}

TEST_F(ConfigurationTest, TestIncludePatternConfig)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config_with_pattern);
  File::Path sub1_path(sub1_config_file);
  sub1_path.write_all(sub1_config);
  File::Path sub2_path(sub2_config_file);
  sub2_path.write_all(sub2_config);

  Log::Streams log;
  XML::Configuration config(toplevel_config_file, log.error);
  ASSERT_TRUE(config.read("toplevel"));

  config.process_includes();
  EXPECT_EQ("bar", config["element/@attr"]);
  EXPECT_EQ("bar2", config["element2/@attr2"]);
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

