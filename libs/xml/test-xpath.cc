//==========================================================================
// ObTools::XML: test-xpath.cc
//
// Test harness for ObTools XPath processor using gtest
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

TEST(XPathTest, TestBasicPath)
{
  string xml="<root><foo><bar attr='wombat'/></foo></root>\n";
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



} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

