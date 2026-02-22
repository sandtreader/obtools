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

//==========================================================================
// read_text tests
//==========================================================================

TEST_F(ConfigurationTest, TestReadText)
{
  ostringstream err;
  XML::Configuration config(err);
  ASSERT_TRUE(config.read_text("<root><item>hello</item></root>"));
  EXPECT_EQ("hello", config["item"]);
}

TEST_F(ConfigurationTest, TestReadTextWithExpectedRoot)
{
  ostringstream err;
  XML::Configuration config(err);
  ASSERT_TRUE(config.read_text("<config><val>1</val></config>", "config"));
  EXPECT_EQ("1", config["val"]);
}

TEST_F(ConfigurationTest, TestReadTextWithWrongRoot)
{
  ostringstream err;
  XML::Configuration config(err);
  EXPECT_FALSE(config.read_text("<wrong><val>1</val></wrong>", "expected"));
  EXPECT_NE(string::npos, err.str().find("Bad root"));
}

TEST_F(ConfigurationTest, TestReadTextInvalidXml)
{
  ostringstream err;
  XML::Configuration config(err);
  EXPECT_FALSE(config.read_text("<unclosed>", "root"));
}

//==========================================================================
// read with wrong root name
//==========================================================================

TEST_F(ConfigurationTest, TestReadWithWrongRootName)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config);

  ostringstream err;
  XML::Configuration config(toplevel_config_file, err);
  EXPECT_FALSE(config.read("wrong_name"));
  EXPECT_NE(string::npos, err.str().find("Bad root"));
}

TEST_F(ConfigurationTest, TestReadFileNotFound)
{
  ostringstream err;
  XML::Configuration config("/tmp/nonexistent-file.xml", err);
  EXPECT_FALSE(config.read());
}

//==========================================================================
// reload test
//==========================================================================

TEST_F(ConfigurationTest, TestReload)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config);

  ostringstream err;
  XML::Configuration config(toplevel_config_file, err);
  ASSERT_TRUE(config.read("toplevel"));
  EXPECT_EQ("foo", config["element/@attr"]);

  // Change the file
  toplevel_path.write_all(
    "<toplevel><element attr=\"bar\"/></toplevel>");

  ASSERT_TRUE(config.reload());
  EXPECT_EQ("bar", config["element/@attr"]);
}

//==========================================================================
// superimpose_file error paths
//==========================================================================

TEST_F(ConfigurationTest, TestSuperimposeFileWrongRoot)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config);
  File::Path sub_path(sub1_config_file);
  sub_path.write_all("<wrong_root><x/></wrong_root>");

  ostringstream err;
  XML::Configuration config(toplevel_config_file, err);
  ASSERT_TRUE(config.read("toplevel"));
  config.superimpose_file(sub1_config_file);
  EXPECT_NE(string::npos, err.str().find("wrong top-level element"));
}

TEST_F(ConfigurationTest, TestSuperimposeFileNotFound)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config);

  ostringstream err;
  XML::Configuration config(toplevel_config_file, err);
  ASSERT_TRUE(config.read("toplevel"));
  config.superimpose_file("/tmp/nonexistent.xml");
  EXPECT_NE(string::npos, err.str().find("Can't read"));
}

//==========================================================================
// Typed getter tests via Configuration
//==========================================================================

class ConfigGetterTest : public ::testing::Test
{
protected:
  ostringstream err;
  XML::Configuration config{err};

  void SetUp() override
  {
    ASSERT_TRUE(config.read_text(
      "<root>"
      "  <name>TestServer</name>"
      "  <debug>true</debug>"
      "  <count>42</count>"
      "  <hexval>ff</hexval>"
      "  <bignum>5000000000</bignum>"
      "  <bighex>ffffffff00</bighex>"
      "  <ratio>3.14</ratio>"
      "  <item>one</item>"
      "  <item>two</item>"
      "  <item>three</item>"
      "  <entry name=\"a\">alpha</entry>"
      "  <entry name=\"b\">beta</entry>"
      "</root>"));
  }
};

TEST_F(ConfigGetterTest, TestGetElement)
{
  XML::Element *e = config.get_element("name");
  ASSERT_NE(nullptr, e);
  EXPECT_EQ("name", e->name);
  EXPECT_EQ("TestServer", e->content);
}

TEST_F(ConfigGetterTest, TestGetValueBool)
{
  EXPECT_TRUE(config.get_value_bool("debug"));
  EXPECT_FALSE(config.get_value_bool("missing"));
  EXPECT_TRUE(config.get_value_bool("missing", true));
}

TEST_F(ConfigGetterTest, TestGetValueInt)
{
  EXPECT_EQ(42, config.get_value_int("count"));
  EXPECT_EQ(0, config.get_value_int("missing"));
  EXPECT_EQ(99, config.get_value_int("missing", 99));
}

TEST_F(ConfigGetterTest, TestGetValueHex)
{
  EXPECT_EQ(255u, config.get_value_hex("hexval"));
  EXPECT_EQ(0u, config.get_value_hex("missing"));
  EXPECT_EQ(16u, config.get_value_hex("missing", 16));
}

TEST_F(ConfigGetterTest, TestGetValueInt64)
{
  EXPECT_EQ(5000000000ULL, config.get_value_int64("bignum"));
  EXPECT_EQ(0ULL, config.get_value_int64("missing"));
  EXPECT_EQ(99ULL, config.get_value_int64("missing", 99));
}

TEST_F(ConfigGetterTest, TestGetValueHex64)
{
  EXPECT_EQ(0xffffffff00ULL, config.get_value_hex64("bighex"));
  EXPECT_EQ(0ULL, config.get_value_hex64("missing"));
  EXPECT_EQ(42ULL, config.get_value_hex64("missing", 42));
}

TEST_F(ConfigGetterTest, TestGetValueReal)
{
  EXPECT_DOUBLE_EQ(3.14, config.get_value_real("ratio"));
  EXPECT_DOUBLE_EQ(0.0, config.get_value_real("missing"));
  EXPECT_DOUBLE_EQ(1.5, config.get_value_real("missing", 1.5));
}

TEST_F(ConfigGetterTest, TestGetValues)
{
  auto values = config.get_values("item");
  ASSERT_EQ(3, static_cast<int>(values.size()));
  auto it = values.begin();
  EXPECT_EQ("one", *it++);
  EXPECT_EQ("two", *it++);
  EXPECT_EQ("three", *it++);
}

TEST_F(ConfigGetterTest, TestGetMap)
{
  auto m = config.get_map("entry");
  ASSERT_EQ(2, static_cast<int>(m.size()));
  EXPECT_EQ("alpha", m["a"]);
  EXPECT_EQ("beta", m["b"]);
}

//==========================================================================
// Typed setter tests via Configuration
//==========================================================================

TEST_F(ConfigGetterTest, TestSetValue)
{
  EXPECT_TRUE(config.set_value("name", "changed"));
  EXPECT_EQ("changed", config["name"]);
}

TEST_F(ConfigGetterTest, TestSetValueBool)
{
  EXPECT_TRUE(config.set_value_bool("debug", false));
  EXPECT_FALSE(config.get_value_bool("debug"));
}

TEST_F(ConfigGetterTest, TestSetValueInt)
{
  EXPECT_TRUE(config.set_value_int("count", 100));
  EXPECT_EQ(100, config.get_value_int("count"));
}

TEST_F(ConfigGetterTest, TestSetValueHex)
{
  EXPECT_TRUE(config.set_value_hex("hexval", 0xab));
  EXPECT_EQ(0xabu, config.get_value_hex("hexval"));
}

TEST_F(ConfigGetterTest, TestSetValueInt64)
{
  EXPECT_TRUE(config.set_value_int64("bignum", 9000000000ULL));
  EXPECT_EQ(9000000000ULL, config.get_value_int64("bignum"));
}

TEST_F(ConfigGetterTest, TestSetValueHex64)
{
  EXPECT_TRUE(config.set_value_hex64("bighex", 0xabcdef0123ULL));
  EXPECT_EQ(0xabcdef0123ULL, config.get_value_hex64("bighex"));
}

TEST_F(ConfigGetterTest, TestSetValueReal)
{
  EXPECT_TRUE(config.set_value_real("ratio", 2.718));
  EXPECT_DOUBLE_EQ(2.718, config.get_value_real("ratio"));
}

//==========================================================================
// Structural modification via Configuration
//==========================================================================

TEST_F(ConfigGetterTest, TestDeleteElements)
{
  EXPECT_TRUE(config.delete_elements("item"));
  auto values = config.get_values("item");
  EXPECT_TRUE(values.empty());
}

TEST_F(ConfigGetterTest, TestAddElementByPointer)
{
  auto *ne = new XML::Element("added", "content");
  EXPECT_TRUE(config.add_element("", ne));
  EXPECT_EQ("content", config["added"]);
}

TEST_F(ConfigGetterTest, TestAddElementByName)
{
  XML::Element *ne = config.add_element("", "new_child");
  ASSERT_NE(nullptr, ne);
  EXPECT_EQ("new_child", ne->name);
}

TEST_F(ConfigGetterTest, TestEnsurePath)
{
  XML::Element *e = config.ensure_path("new/deep/path");
  ASSERT_NE(nullptr, e);
  EXPECT_EQ("path", e->name);
}

TEST_F(ConfigGetterTest, TestReplaceElement)
{
  auto *ne = new XML::Element("name", "replaced");
  EXPECT_TRUE(config.replace_element("name", ne));
  EXPECT_EQ("replaced", config["name"]);
}

TEST_F(ConfigGetterTest, TestReplaceRoot)
{
  XML::Element *new_root = config.replace_root("new_root");
  ASSERT_NE(nullptr, new_root);
  EXPECT_EQ("new_root", new_root->name);
  EXPECT_EQ("new_root", config.get_root().name);
}

//==========================================================================
// Write test
//==========================================================================

TEST_F(ConfigurationTest, TestWrite)
{
  File::Path toplevel_path(toplevel_config_file);
  toplevel_path.write_all(toplevel_config);

  ostringstream err;
  XML::Configuration config(toplevel_config_file, err);
  ASSERT_TRUE(config.read("toplevel"));

  // Modify and write back
  config.set_value("element/@attr", "modified");
  ASSERT_TRUE(config.write());

  // Read back and verify
  XML::Configuration config2(toplevel_config_file, err);
  ASSERT_TRUE(config2.read("toplevel"));
  EXPECT_EQ("modified", config2["element/@attr"]);
}

TEST_F(ConfigurationTest, TestWriteNoFilename)
{
  ostringstream err;
  XML::Configuration config(err);
  config.read_text("<root><val>1</val></root>");
  // config has no filename, so write should fail
  config.add_file("");
  EXPECT_FALSE(config.write());
  EXPECT_NE(string::npos, err.str().find("no filename"));
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

