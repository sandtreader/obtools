//==========================================================================
// ObTools::XML: test-xpath.cc
//
// Comprehensive test harness for ObTools XPath processor using gtest
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
#include <iostream>
#include <sstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// Test fixture with a reusable XML document
//==========================================================================

class XPathTest : public ::testing::Test
{
protected:
  XML::Parser parser;
  XML::Element *root{nullptr};

  void SetUp() override
  {
    string xml =
      "<config version=\"1.1\">\n"
      "  <directory>test</directory>\n"
      "  <server host=\"localhost\" port=\"8080\">\n"
      "    <name>TestServer</name>\n"
      "    <debug>true</debug>\n"
      "    <timeout>30</timeout>\n"
      "    <ratio>0.75</ratio>\n"
      "    <hexval>ff</hexval>\n"
      "    <bignum>5000000000</bignum>\n"
      "    <bighex>ffffffff00</bighex>\n"
      "  </server>\n"
      "  <item id=\"1\">first</item>\n"
      "  <item id=\"2\">second</item>\n"
      "  <item id=\"3\">third</item>\n"
      "  <nested>\n"
      "    <deep>\n"
      "      <value>found</value>\n"
      "    </deep>\n"
      "  </nested>\n"
      "</config>\n";

    parser.read_from(xml);
    root = &parser.get_root();
  }
};

//==========================================================================
// Basic read operations
//==========================================================================

TEST_F(XPathTest, TestGetElementSimple)
{
  XML::XPathProcessor xpath(*root);
  XML::Element *e = xpath.get_element("server");
  ASSERT_NE(nullptr, e);
  EXPECT_EQ("server", e->name);
}

TEST_F(XPathTest, TestGetElementDeepPath)
{
  XML::XPathProcessor xpath(*root);
  XML::Element *e = xpath.get_element("nested/deep/value");
  ASSERT_NE(nullptr, e);
  EXPECT_EQ("value", e->name);
  EXPECT_EQ("found", e->content);
}

TEST_F(XPathTest, TestGetElementNotFound)
{
  XML::XPathProcessor xpath(*root);
  XML::Element *e = xpath.get_element("nonexistent");
  EXPECT_EQ(nullptr, e);
}

TEST_F(XPathTest, TestGetElementsMultiple)
{
  XML::XPathProcessor xpath(*root);
  auto items = xpath.get_elements("item");
  EXPECT_EQ(3, static_cast<int>(items.size()));
}

TEST_F(XPathTest, TestGetElementsSingle)
{
  XML::XPathProcessor xpath(*root);
  auto servers = xpath.get_elements("server");
  EXPECT_EQ(1, static_cast<int>(servers.size()));
}

TEST_F(XPathTest, TestGetElementsNone)
{
  XML::XPathProcessor xpath(*root);
  auto missing = xpath.get_elements("nonexistent");
  EXPECT_EQ(0, static_cast<int>(missing.size()));
}

//==========================================================================
// Value fetch tests
//==========================================================================

TEST_F(XPathTest, TestGetValueContent)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("test", xpath.get_value("directory"));
}

TEST_F(XPathTest, TestGetValueAttribute)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("1.1", xpath.get_value("@version"));
}

TEST_F(XPathTest, TestGetValueDeepAttribute)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("localhost", xpath.get_value("server/@host"));
}

TEST_F(XPathTest, TestGetValueDefault)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("", xpath.get_value("nonexistent"));
  EXPECT_EQ("default", xpath.get_value("nonexistent", "default"));
}

TEST_F(XPathTest, TestGetValueDefaultMissingAttr)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("none", xpath.get_value("server/@missing", "none"));
}

TEST_F(XPathTest, TestOperatorBracket)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("test", xpath["directory"]);
  EXPECT_EQ("1.1", xpath["@version"]);
}

TEST_F(XPathTest, TestGetValueNested)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("found", xpath["nested/deep/value"]);
}

//==========================================================================
// Typed value fetch tests
//==========================================================================

TEST_F(XPathTest, TestGetValueBool)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.get_value_bool("server/debug"));
  EXPECT_FALSE(xpath.get_value_bool("nonexistent"));
  EXPECT_TRUE(xpath.get_value_bool("nonexistent", true));
}

TEST_F(XPathTest, TestGetValueInt)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ(30, xpath.get_value_int("server/timeout"));
  EXPECT_EQ(0, xpath.get_value_int("nonexistent"));
  EXPECT_EQ(42, xpath.get_value_int("nonexistent", 42));
}

TEST_F(XPathTest, TestGetValueHex)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ(255u, xpath.get_value_hex("server/hexval"));
  EXPECT_EQ(0u, xpath.get_value_hex("nonexistent"));
  EXPECT_EQ(16u, xpath.get_value_hex("nonexistent", 16));
}

TEST_F(XPathTest, TestGetValueInt64)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ(5000000000ULL, xpath.get_value_int64("server/bignum"));
  EXPECT_EQ(0ULL, xpath.get_value_int64("nonexistent"));
  EXPECT_EQ(99ULL, xpath.get_value_int64("nonexistent", 99));
}

TEST_F(XPathTest, TestGetValueHex64)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ(0xffffffff00ULL, xpath.get_value_hex64("server/bighex"));
  EXPECT_EQ(0ULL, xpath.get_value_hex64("nonexistent"));
}

TEST_F(XPathTest, TestGetValueReal)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_DOUBLE_EQ(0.75, xpath.get_value_real("server/ratio"));
  EXPECT_DOUBLE_EQ(0.0, xpath.get_value_real("nonexistent"));
  EXPECT_DOUBLE_EQ(1.5, xpath.get_value_real("nonexistent", 1.5));
}

//==========================================================================
// Indexed path tests
//==========================================================================

TEST_F(XPathTest, TestIndexedAccess)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("first", xpath["item"]);
  EXPECT_EQ("second", xpath.get_value("item[2]"));
  EXPECT_EQ("third", xpath.get_value("item[3]"));
}

TEST_F(XPathTest, TestIndexedAccessOutOfRange)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("", xpath.get_value("item[4]"));
}

TEST_F(XPathTest, TestIndexedAccessAttribute)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("1", xpath.get_value("item/@id"));
  EXPECT_EQ("2", xpath.get_value("item[2]/@id"));
  EXPECT_EQ("3", xpath.get_value("item[3]/@id"));
}

//==========================================================================
// Root-relative paths
//==========================================================================

TEST_F(XPathTest, TestAbsolutePathAttribute)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("1.1", xpath["/@version"]);
}

TEST_F(XPathTest, TestAbsolutePathContent)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_EQ("test", xpath["/directory"]);
}

TEST_F(XPathTest, TestRootContent)
{
  XML::Element simple("root", "content");
  XML::XPathProcessor xpath(simple);
  EXPECT_EQ("content", xpath["/"]);
}

//==========================================================================
// XPathProcessor set value tests
//==========================================================================

TEST_F(XPathTest, TestSetValue)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value("directory", "changed"));
  EXPECT_EQ("changed", xpath["directory"]);
}

TEST_F(XPathTest, TestSetValueAttribute)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value("server/@host", "newhost"));
  EXPECT_EQ("newhost", xpath["server/@host"]);
}

TEST_F(XPathTest, TestSetValueBool)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value_bool("server/debug", false));
  EXPECT_FALSE(xpath.get_value_bool("server/debug"));
}

TEST_F(XPathTest, TestSetValueInt)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value_int("server/timeout", 60));
  EXPECT_EQ(60, xpath.get_value_int("server/timeout"));
}

TEST_F(XPathTest, TestSetValueHex)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value_hex("server/hexval", 0xab));
  EXPECT_EQ(0xabu, xpath.get_value_hex("server/hexval"));
}

TEST_F(XPathTest, TestSetValueInt64)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value_int64("server/bignum", 9000000000ULL));
  EXPECT_EQ(9000000000ULL, xpath.get_value_int64("server/bignum"));
}

TEST_F(XPathTest, TestSetValueHex64)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value_hex64("server/bighex", 0xabcdef0123ULL));
  EXPECT_EQ(0xabcdef0123ULL, xpath.get_value_hex64("server/bighex"));
}

TEST_F(XPathTest, TestSetValueReal)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_TRUE(xpath.set_value_real("server/ratio", 3.14));
  EXPECT_DOUBLE_EQ(3.14, xpath.get_value_real("server/ratio"));
}

TEST_F(XPathTest, TestSetValueOnNonexistent)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_FALSE(xpath.set_value("nonexistent", "value"));
}

//==========================================================================
// XPathProcessor structural modification tests
//==========================================================================

TEST_F(XPathTest, TestDeleteElements)
{
  XML::XPathProcessor xpath(*root);
  ASSERT_EQ(3, static_cast<int>(xpath.get_elements("item").size()));

  EXPECT_TRUE(xpath.delete_elements("item"));
  EXPECT_EQ(0, static_cast<int>(xpath.get_elements("item").size()));
}

TEST_F(XPathTest, TestDeleteElementsNonexistent)
{
  XML::XPathProcessor xpath(*root);
  EXPECT_FALSE(xpath.delete_elements("nonexistent"));
}

TEST_F(XPathTest, TestAddElementByPointer)
{
  XML::XPathProcessor xpath(*root);
  auto *ne = new XML::Element("added", "content");
  EXPECT_TRUE(xpath.add_element("server", ne));
  EXPECT_EQ("content", xpath["server/added"]);
}

TEST_F(XPathTest, TestAddElementByName)
{
  XML::XPathProcessor xpath(*root);
  XML::Element *ne = xpath.add_element("server", "new_child");
  ASSERT_NE(nullptr, ne);
  EXPECT_EQ("new_child", ne->name);
  EXPECT_TRUE(root->get_child("server").get_child("new_child").valid());
}

TEST_F(XPathTest, TestAddElementToNonexistent)
{
  XML::XPathProcessor xpath(*root);
  auto *ne = new XML::Element("child");
  EXPECT_FALSE(xpath.add_element("nonexistent", ne));
  delete ne;
}

TEST_F(XPathTest, TestEnsurePath)
{
  XML::XPathProcessor xpath(*root);
  XML::Element *e = xpath.ensure_path("new/deep/path");

  ASSERT_NE(nullptr, e);
  EXPECT_EQ("path", e->name);
  EXPECT_EQ("found", xpath["nested/deep/value"]);
  EXPECT_TRUE(root->get_child("new").valid());
}

TEST_F(XPathTest, TestEnsurePathExisting)
{
  XML::XPathProcessor xpath(*root);
  XML::Element *e = xpath.ensure_path("server/name");
  ASSERT_NE(nullptr, e);
  EXPECT_EQ("TestServer", e->content);
}

TEST_F(XPathTest, TestReplaceElement)
{
  XML::XPathProcessor xpath(*root);
  auto *ne = new XML::Element("directory", "replaced");

  EXPECT_TRUE(xpath.replace_element("directory", ne));
  EXPECT_EQ("replaced", xpath["directory"]);
}

TEST_F(XPathTest, TestReplaceElementNonexistent)
{
  XML::XPathProcessor xpath(*root);
  auto *ne = new XML::Element("foo");

  EXPECT_FALSE(xpath.replace_element("nonexistent", ne));
  delete ne;
}

//==========================================================================
// ConstXPathProcessor tests
//==========================================================================

TEST_F(XPathTest, TestConstXPathProcessorRead)
{
  const XML::Element& const_root = *root;
  XML::ConstXPathProcessor xpath(const_root);

  EXPECT_EQ("test", xpath["directory"]);
  EXPECT_EQ("1.1", xpath["@version"]);
  EXPECT_EQ("localhost", xpath["server/@host"]);
}

TEST_F(XPathTest, TestConstXPathProcessorTypedValues)
{
  const XML::Element& const_root = *root;
  XML::ConstXPathProcessor xpath(const_root);

  EXPECT_TRUE(xpath.get_value_bool("server/debug"));
  EXPECT_EQ(30, xpath.get_value_int("server/timeout"));
  EXPECT_DOUBLE_EQ(0.75, xpath.get_value_real("server/ratio"));
}

TEST_F(XPathTest, TestConstXPathGetElements)
{
  const XML::Element& const_root = *root;
  XML::ConstXPathProcessor xpath(const_root);

  auto items = xpath.get_elements("item");
  EXPECT_EQ(3, static_cast<int>(items.size()));
}

//==========================================================================
// Default-constructed processor tests
//==========================================================================

TEST(XPathDefaultTest, TestDefaultXPathProcessor)
{
  XML::XPathProcessor xpath;
  EXPECT_EQ("", xpath["anything"]);
  EXPECT_EQ(nullptr, xpath.get_element("anything"));
}

TEST(XPathDefaultTest, TestDefaultConstXPathProcessor)
{
  XML::ConstXPathProcessor xpath;
  EXPECT_EQ("", xpath["anything"]);
  EXPECT_EQ(nullptr, xpath.get_element("anything"));
}

//==========================================================================
// Standalone tests (no fixture)
//==========================================================================

TEST(XPathStandaloneTest, TestBasicPath)
{
  string xml = "<root><foo><bar attr='wombat'/></foo></root>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  XML::XPathProcessor xpath(e);

  XML::Element *foo = xpath.get_element("foo");
  ASSERT_TRUE(foo);
  EXPECT_EQ("foo", foo->name);

  XML::Element *bar = xpath.get_element("foo/bar");
  ASSERT_TRUE(bar);
  EXPECT_EQ("bar", bar->name);

  EXPECT_EQ("wombat", xpath["foo/bar/@attr"]);
}

TEST(XPathStandaloneTest, TestAttributeOnRootElement)
{
  string xml = "<root attr1=\"value1\" attr2=\"value2\"/>";
  XML::Parser parser;
  parser.read_from(xml);
  XML::XPathProcessor xpath(parser.get_root());

  EXPECT_EQ("value1", xpath["@attr1"]);
  EXPECT_EQ("value2", xpath["@attr2"]);
  EXPECT_EQ("value1", xpath["/@attr1"]);
}

TEST(XPathStandaloneTest, TestSetValueCreatesContent)
{
  XML::Element root("root");
  root.add("child");
  XML::XPathProcessor xpath(root);

  xpath.set_value("child", "new_content");
  EXPECT_EQ("new_content", xpath["child"]);
}

TEST(XPathStandaloneTest, TestSetRootAttribute)
{
  XML::Element root("root");
  XML::XPathProcessor xpath(root);

  xpath.set_value("@attr", "value");
  EXPECT_EQ("value", xpath["@attr"]);
}

TEST(XPathStandaloneTest, TestDeleteAndReAdd)
{
  XML::Element root("root");
  root.add("child", "original");
  XML::XPathProcessor xpath(root);

  xpath.delete_elements("child");
  EXPECT_EQ("", xpath["child"]);

  xpath.add_element("", "child");
  xpath.set_value("child", "recreated");
  EXPECT_EQ("recreated", xpath["child"]);
}

TEST(XPathStandaloneTest, TestEnsurePathFromScratch)
{
  XML::Element root("root");
  XML::XPathProcessor xpath(root);

  XML::Element *leaf = xpath.ensure_path("a/b/c");
  ASSERT_NE(nullptr, leaf);
  EXPECT_EQ("c", leaf->name);

  EXPECT_TRUE(root.get_child("a").valid());
  EXPECT_TRUE(root.get_child("a").get_child("b").valid());
  EXPECT_TRUE(root.get_child("a").get_child("b").get_child("c").valid());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
