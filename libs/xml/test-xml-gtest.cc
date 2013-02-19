//==========================================================================
// ObTools::XML: test-xml-gtest.cc
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

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

