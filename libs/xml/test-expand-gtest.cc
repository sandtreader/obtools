//==========================================================================
// ObTools::XML: test-expand-gtest.cc
//
// Comprehensive test harness for ObTools XML Expander using gtest
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-xml.h"
#include <iostream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//==========================================================================
// Helper to parse an XML template from string (with preserved whitespace)
//==========================================================================

static XML::Element& parse_template(XML::Parser& parser, const string& xml)
{
  parser.read_from(xml);
  return parser.get_root();
}

//==========================================================================
// expand:replace tests
//==========================================================================

TEST(ExpanderTest, TestReplaceWithValue)
{
  string tmpl_xml =
    "<t><expand:replace value=\"greeting\"/></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("greeting", "Hello World");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("Hello World", result);
}

TEST(ExpanderTest, TestReplaceWithVar)
{
  string tmpl_xml =
    "<t>"
    "<expand:set var=\"myvar\">set_value</expand:set>"
    "<expand:replace var=\"myvar\"/>"
    "</t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("set_value", result);
}

TEST(ExpanderTest, TestReplaceWithMissingValue)
{
  string tmpl_xml =
    "<t><expand:replace value=\"missing\"/></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

TEST(ExpanderTest, TestReplaceWithAttribute)
{
  string tmpl_xml =
    "<t><expand:replace value=\"item/@name\"/></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("item", "name", "wombat");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("wombat", result);
}

//==========================================================================
// expand:if tests
//==========================================================================

TEST(ExpanderTest, TestIfTrueExpands)
{
  string tmpl_xml =
    "<t><expand:if value=\"flag\">included</expand:if></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("flag", "true");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("included", result);
}

TEST(ExpanderTest, TestIfYesExpands)
{
  string tmpl_xml =
    "<t><expand:if value=\"flag\">included</expand:if></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("flag", "Yes");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("included", result);
}

TEST(ExpanderTest, TestIf1Expands)
{
  string tmpl_xml =
    "<t><expand:if value=\"flag\">included</expand:if></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("flag", "1");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("included", result);
}

TEST(ExpanderTest, TestIfFalseExcludes)
{
  string tmpl_xml =
    "<t><expand:if value=\"flag\">excluded</expand:if></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("flag", "false");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

TEST(ExpanderTest, TestIfMissingExcludes)
{
  string tmpl_xml =
    "<t><expand:if value=\"flag\">excluded</expand:if></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

TEST(ExpanderTest, TestIfWithVar)
{
  string tmpl_xml =
    "<t>"
    "<expand:set var=\"v\">true</expand:set>"
    "<expand:if var=\"v\">yes</expand:if>"
    "</t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("yes", result);
}

//==========================================================================
// expand:unless tests
//==========================================================================

TEST(ExpanderTest, TestUnlessFalseExpands)
{
  string tmpl_xml =
    "<t><expand:unless value=\"flag\">included</expand:unless></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("flag", "false");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("included", result);
}

TEST(ExpanderTest, TestUnlessTrueExcludes)
{
  string tmpl_xml =
    "<t><expand:unless value=\"flag\">excluded</expand:unless></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("flag", "true");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

TEST(ExpanderTest, TestUnlessMissingExpands)
{
  string tmpl_xml =
    "<t><expand:unless value=\"flag\">included</expand:unless></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("included", result);
}

//==========================================================================
// expand:ifeq tests
//==========================================================================

TEST(ExpanderTest, TestIfeqMatchExpands)
{
  string tmpl_xml =
    "<t><expand:ifeq value=\"colour\" to=\"red\">match</expand:ifeq></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("colour", "red");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("match", result);
}

TEST(ExpanderTest, TestIfeqNoMatchExcludes)
{
  string tmpl_xml =
    "<t><expand:ifeq value=\"colour\" to=\"red\">no</expand:ifeq></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("colour", "blue");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

TEST(ExpanderTest, TestIfeqCaseSensitive)
{
  string tmpl_xml =
    "<t><expand:ifeq value=\"v\" to=\"Yes\">ok</expand:ifeq></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("v", "yes");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

//==========================================================================
// expand:ifne tests
//==========================================================================

TEST(ExpanderTest, TestIfneNotEqualExpands)
{
  string tmpl_xml =
    "<t><expand:ifne value=\"v\" to=\"red\">ok</expand:ifne></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("v", "blue");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("ok", result);
}

TEST(ExpanderTest, TestIfneEqualExcludes)
{
  string tmpl_xml =
    "<t><expand:ifne value=\"v\" to=\"red\">no</expand:ifne></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("v", "red");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

//==========================================================================
// expand:each tests
//==========================================================================

TEST(ExpanderTest, TestEachLoop)
{
  string tmpl_xml =
    "<t><expand:each element=\"item\">"
    "<expand:replace/>"
    "</expand:each></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("item", "alpha");
  values.add("item", "beta");
  values.add("item", "gamma");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("alphabetagamma", result);
}

TEST(ExpanderTest, TestEachLoopEmpty)
{
  string tmpl_xml =
    "<t><expand:each element=\"item\">x</expand:each></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("", result);
}

TEST(ExpanderTest, TestEachWithAttributeAccess)
{
  string tmpl_xml =
    "<t><expand:each element=\"person\">"
    "<expand:replace value=\"@name\"/>"
    "</expand:each></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("person", "name", "Alice");
  values.add("person", "name", "Bob");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("AliceBob", result);
}

//==========================================================================
// expand:index tests
//==========================================================================

TEST(ExpanderTest, TestIndexDefault)
{
  string tmpl_xml =
    "<t><expand:each element=\"item\">"
    "<expand:index/>"
    "</expand:each></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("item", "a");
  values.add("item", "b");
  values.add("item", "c");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("123", result);
}

TEST(ExpanderTest, TestIndexFromZero)
{
  string tmpl_xml =
    "<t><expand:each element=\"item\">"
    "<expand:index from=\"0\"/>"
    "</expand:each></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("item", "a");
  values.add("item", "b");
  values.add("item", "c");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("012", result);
}

//==========================================================================
// expand:set tests
//==========================================================================

TEST(ExpanderTest, TestSetVariable)
{
  string tmpl_xml =
    "<t>"
    "<expand:set var=\"x\">hello</expand:set>"
    "<expand:replace var=\"x\"/>"
    "</t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("hello", result);
}

TEST(ExpanderTest, TestSetVariableFromExpansion)
{
  string tmpl_xml =
    "<t>"
    "<expand:set var=\"x\"><expand:replace value=\"data\"/></expand:set>"
    "<expand:replace var=\"x\"/>"
    "</t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("data", "dynamic");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("dynamic", result);
}

TEST(ExpanderTest, TestVariableScoping)
{
  string tmpl_xml =
    "<t>"
    "<expand:set var=\"outer\">A</expand:set>"
    "<expand:each element=\"item\">"
      "<expand:set var=\"inner\">B</expand:set>"
    "</expand:each>"
    "<expand:replace var=\"outer\"/>"
    "</t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("item", "x");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("A", result);
}

//==========================================================================
// Verbatim element passthrough tests
//==========================================================================

TEST(ExpanderTest, TestVerbatimElementEmpty)
{
  string tmpl_xml =
    "<t><img src=\"foo\"/></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("<img src=\"foo\"/>\n", result);
}

TEST(ExpanderTest, TestVerbatimElementWithChildren)
{
  string tmpl_xml =
    "<t><b>bold</b></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("<b>bold</b>", result);
}

//==========================================================================
// Combined / integration tests
//==========================================================================

TEST(ExpanderTest, TestMixedContent)
{
  string tmpl_xml =
    "<t>Name: <expand:replace value=\"name\"/></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("name", "Alice");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("Name: Alice", result);
}

TEST(ExpanderTest, TestIfWithNestedReplace)
{
  string tmpl_xml =
    "<t><expand:if value=\"show\">"
    "Value: <expand:replace value=\"data\"/>"
    "</expand:if></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("show", "yes");
  values.add("data", "42");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("Value: 42", result);
}

TEST(ExpanderTest, TestEachWithIndexAndReplace)
{
  string tmpl_xml =
    "<t><expand:each element=\"item\">"
    "<expand:index/>:<expand:replace/> "
    "</expand:each></t>";
  XML::Parser p(XML::PARSER_PRESERVE_WHITESPACE);
  XML::Element& tmpl = parse_template(p, tmpl_xml);

  XML::Element values("values");
  values.add("item", "a");
  values.add("item", "b");

  XML::Expander expander(tmpl);
  string result = expander.expand(values);
  EXPECT_EQ("1:a 2:b ", result);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
