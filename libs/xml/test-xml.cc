//==========================================================================
// ObTools::XML: test-xml.cc
//
// Test harness for ObTools XML Parser using gtest
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
#include <iostream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(XMLTest, TestBasicParse)
{
  string xml="<foo><bar/></foo>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ(1, e.children.size());
  EXPECT_FALSE(!e.get_child("bar"));
}

TEST(XMLTest, TestContentOptimisation)
{
  string xml="<foo>content</foo>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("content", *e);
}

TEST(XMLTest, TestContentOptimisationWithComment)
{
  string xml="<foo>content<!--comment-->more</foo>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("content more", *e);
}

TEST(XMLTest, TestAttributeEscaping)
{
  XML::Element e("test");
  e.set_attr("normal", "foo");
  e.set_attr("lt", "<foo");
  e.set_attr("gt", ">foo");
  e.set_attr("amp", "&foo");
  e.set_attr("dquot", "\"foo");
  e.set_attr("squot", "'foo");
  e.set_attr("bothquot", "'\"foo");

  string streamed_xml = e.to_string();
  ASSERT_EQ("<test amp=\"&amp;foo\" bothquot=\"'&quot;foo\" dquot='\"foo' gt=\"&gt;foo\" lt=\"&lt;foo\" normal=\"foo\" squot=\"'foo\"/>\n", streamed_xml);
}

TEST(XMLTest, TestSuperimpose)
{
  XML::Element a("root");
  a.set_attr("name", "foo");
  XML::Element &a_c1 = a.add("child", "id", "1");
  XML::Element &a_c2 = a.add("child", "id", "2");
  a_c1.set_attr("name", "pickle");
  a_c1.add("colour", "yellow");
  a_c2.set_attr("name", "sprout");
  a_c2.add("colour", "green");
  XML::Element b("root");
  b.set_attr("name", "bar");
  XML::Element &b_c1 = b.add("child", "id", "1");
  XML::Element &b_c3 = b.add("child", "id", "3");
  b_c1.set_attr("name", "apricot");
  b_c3.set_attr("name", "plum");
  b_c1.add("colour", "orange");
  b_c3.add("colour", "purple");

  a.superimpose(b, "id");

  string expected = "<root name=\"bar\">\n"
                    "  <child id=\"1\" name=\"apricot\">\n"
                    "    <colour>orange</colour>\n"
                    "  </child>\n"
                    "  <child id=\"2\" name=\"sprout\">\n"
                    "    <colour>green</colour>\n"
                    "  </child>\n"
                    "  <child id=\"3\" name=\"plum\">\n"
                    "    <colour>purple</colour>\n"
                    "  </child>\n"
                    "</root>\n";
  ASSERT_EQ(expected, a.to_string());
}

TEST(XMLTest, TestGetChildElement)
{
  string xml="<foo>text<bar/> <splat/></foo>\n";
  XML::Parser parser(XML::PARSER_PRESERVE_WHITESPACE);
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  XML::Element& bar = e.get_child_element();
  ASSERT_EQ("bar", bar.name);
  XML::Element& splat = e.get_child_element(1);
  ASSERT_EQ("splat", splat.name);
}

TEST(XMLTest, TestIgnoreBOM)
{
  string xml="\xef\xbb\xbf<foo/>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
}

TEST(XMLTest, TestFailOnBogusBOM)
{
  string xml="\xef\xbf\xbb<foo/>\n";
  XML::Parser parser;
  ASSERT_THROW(parser.read_from(xml), XML::ParseFailed);
}

TEST(XMLTest, TestGetPrefixedAttributes)
{
  XML::Element e("foo");
  e.set_attr("prefix-x", "X");
  e.set_attr("prefix-y", "Y");
  e.set_attr("ignore", "bar");

  auto attrs = e.get_attrs_with_prefix("prefix-");
  ASSERT_EQ(2, attrs.size());
  EXPECT_EQ("X", attrs["x"]);
  EXPECT_EQ("Y", attrs["y"]);
}

TEST(XMLTest, TestAddPrefix)
{
  XML::Element a("root");
  XML::Element &a_c1 = a.add("child");
  XML::Element &a_c2 = a.add("foo:child");

  a.add_prefix("foo:");

  ASSERT_EQ("foo:root", a.name);
  ASSERT_EQ("foo:child", a_c1.name);
  ASSERT_EQ("foo:child", a_c2.name);
}

TEST(XMLTest, TestRemovePrefix)
{
  XML::Element a("foo:root");
  XML::Element &a_c1 = a.add("foo:child");
  XML::Element &a_c2 = a.add("bar:child");
  XML::Element &a_c3 = a.add("foo:");

  a.remove_prefix("foo:");

  ASSERT_EQ("root", a.name);
  ASSERT_EQ("child", a_c1.name);
  ASSERT_EQ("bar:child", a_c2.name);
  ASSERT_EQ("", a_c3.name);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

