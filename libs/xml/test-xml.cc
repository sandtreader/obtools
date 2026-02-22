//==========================================================================
// ObTools::XML: test-xml.cc
//
// Comprehensive test harness for ObTools XML Element and Parser using gtest
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
// Element construction tests
//==========================================================================

TEST(ElementConstructorTest, TestDefaultConstructor)
{
  XML::Element e;
  EXPECT_EQ("", e.name);
  EXPECT_EQ("", e.content);
  EXPECT_TRUE(e.attrs.empty());
  EXPECT_TRUE(e.children.empty());
  EXPECT_EQ(nullptr, e.parent);
  EXPECT_EQ(0, e.line);
}

TEST(ElementConstructorTest, TestNameConstructor)
{
  XML::Element e("foo");
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("", e.content);
  EXPECT_TRUE(e.attrs.empty());
  EXPECT_TRUE(e.children.empty());
}

TEST(ElementConstructorTest, TestNameContentConstructor)
{
  XML::Element e("foo", "bar");
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("bar", e.content);
  EXPECT_TRUE(e.attrs.empty());
}

TEST(ElementConstructorTest, TestNameAttrValueConstructor)
{
  XML::Element e("foo", "id", "42");
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("", e.content);
  EXPECT_EQ("42", e.get_attr("id"));
}

TEST(ElementConstructorTest, TestNameAttrValueContentConstructor)
{
  XML::Element e("foo", "id", "42", "content");
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("content", e.content);
  EXPECT_EQ("42", e.get_attr("id"));
}

//==========================================================================
// Element validity tests
//==========================================================================

TEST(ElementValidityTest, TestElementNoneIsInvalid)
{
  EXPECT_FALSE(XML::Element::none.valid());
  EXPECT_TRUE(!XML::Element::none);
  EXPECT_FALSE(static_cast<bool>(XML::Element::none));
}

TEST(ElementValidityTest, TestNormalElementIsValid)
{
  XML::Element e("foo");
  EXPECT_TRUE(e.valid());
  EXPECT_FALSE(!e);
  EXPECT_TRUE(static_cast<bool>(e));
}

TEST(ElementValidityTest, TestDefaultElementIsValid)
{
  XML::Element e;
  EXPECT_TRUE(e.valid());
}

//==========================================================================
// Copy tests
//==========================================================================

TEST(ElementCopyTest, TestShallowCopy)
{
  XML::Element src("root", "hello");
  src.set_attr("key", "value");
  src.add("child", "data");

  XML::Element dest;
  src.copy_to(dest);

  EXPECT_EQ("root", dest.name);
  EXPECT_EQ("hello", dest.content);
  EXPECT_EQ("value", dest.get_attr("key"));
  EXPECT_TRUE(dest.children.empty());
}

TEST(ElementCopyTest, TestShallowCopyNewElement)
{
  XML::Element src("root", "id", "1");
  XML::Element *dest = src.copy();

  EXPECT_EQ("root", dest->name);
  EXPECT_EQ("1", dest->get_attr("id"));
  EXPECT_TRUE(dest->children.empty());
  delete dest;
}

TEST(ElementCopyTest, TestDeepCopy)
{
  XML::Element src("root");
  src.set_attr("key", "value");
  XML::Element& child = src.add("child", "data");
  child.add("grandchild");

  XML::Element dest;
  src.deep_copy_to(dest);

  EXPECT_EQ("root", dest.name);
  EXPECT_EQ("value", dest.get_attr("key"));
  ASSERT_EQ(1, static_cast<int>(dest.children.size()));
  XML::Element& dc = dest.get_child("child");
  ASSERT_TRUE(dc.valid());
  EXPECT_EQ("data", dc.content);
  EXPECT_TRUE(dc.get_child("grandchild").valid());
}

TEST(ElementCopyTest, TestDeepCopyNewElement)
{
  XML::Element src("root");
  src.add("child", "data");

  XML::Element *dest = src.deep_copy();
  EXPECT_EQ("root", dest->name);
  ASSERT_EQ(1, static_cast<int>(dest->children.size()));
  EXPECT_EQ("child", dest->get_child().name);
  delete dest;
}

TEST(ElementCopyTest, TestCopyConstructor)
{
  XML::Element src("root");
  src.set_attr("a", "1");
  src.add("child", "text");

  XML::Element copy(src);
  EXPECT_EQ("root", copy.name);
  EXPECT_EQ("1", copy.get_attr("a"));
  EXPECT_TRUE(copy.get_child("child").valid());
  EXPECT_EQ(nullptr, copy.parent);
}

TEST(ElementCopyTest, TestAssignmentOperator)
{
  XML::Element src("root");
  src.add("child");

  XML::Element dest("old");
  dest.add("old-child");

  dest = src;
  EXPECT_EQ("root", dest.name);
  ASSERT_EQ(1, static_cast<int>(dest.children.size()));
  EXPECT_EQ("child", dest.get_child().name);
}

//==========================================================================
// Child access tests
//==========================================================================

TEST(ElementChildTest, TestGetChildByIndex)
{
  XML::Element root("root");
  root.add("first");
  root.add("second");
  root.add("third");

  EXPECT_EQ("first", root.get_child(0).name);
  EXPECT_EQ("second", root.get_child(1).name);
  EXPECT_EQ("third", root.get_child(2).name);
  EXPECT_FALSE(root.get_child(3).valid());
}

TEST(ElementChildTest, TestGetChildByName)
{
  XML::Element root("root");
  root.add("alpha");
  root.add("beta");
  root.add("alpha", "second");

  XML::Element& first_alpha = root.get_child("alpha");
  ASSERT_TRUE(first_alpha.valid());
  EXPECT_EQ("", first_alpha.content);

  XML::Element& second_alpha = root.get_child("alpha", 1);
  ASSERT_TRUE(second_alpha.valid());
  EXPECT_EQ("second", second_alpha.content);

  EXPECT_FALSE(root.get_child("alpha", 2).valid());
  EXPECT_FALSE(root.get_child("gamma").valid());
}

TEST(ElementChildTest, TestGetChildFromEmptyElement)
{
  XML::Element root("root");
  EXPECT_FALSE(root.get_child().valid());
  EXPECT_FALSE(root.get_child("foo").valid());
}

TEST(ElementChildTest, TestGetChildElement)
{
  string xml = "<foo>text<bar/> <splat/></foo>\n";
  XML::Parser parser(XML::PARSER_PRESERVE_WHITESPACE);
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  XML::Element& bar = e.get_child_element();
  ASSERT_EQ("bar", bar.name);
  XML::Element& splat = e.get_child_element(1);
  ASSERT_EQ("splat", splat.name);
  EXPECT_FALSE(e.get_child_element(2).valid());
}

TEST(ElementChildTest, TestMakeChild)
{
  XML::Element root("root");

  XML::Element& child = root.make_child("child");
  EXPECT_EQ("child", child.name);
  EXPECT_EQ(1, static_cast<int>(root.children.size()));

  XML::Element& same_child = root.make_child("child");
  EXPECT_EQ(&child, &same_child);
  EXPECT_EQ(1, static_cast<int>(root.children.size()));
}

TEST(ElementChildTest, TestGetChildren)
{
  XML::Element root("root");
  root.add("item", "one");
  root.add("other", "x");
  root.add("item", "two");
  root.add("item", "three");

  auto items = root.get_children("item");
  ASSERT_EQ(3, static_cast<int>(items.size()));

  auto all = root.get_children();
  EXPECT_EQ(4, static_cast<int>(all.size()));

  auto none = root.get_children("missing");
  EXPECT_EQ(0, static_cast<int>(none.size()));
}

//==========================================================================
// Descendant tests
//==========================================================================

TEST(ElementDescendantTest, TestGetDescendant)
{
  XML::Element root("root");
  XML::Element& mid = root.add("middle");
  mid.add("target", "found");

  const XML::Element& target = root.get_descendant("target");
  ASSERT_TRUE(target.valid());
  EXPECT_EQ("found", target.content);

  EXPECT_FALSE(root.get_descendant("nonexistent").valid());
}

TEST(ElementDescendantTest, TestGetDescendants)
{
  XML::Element root("root");
  root.add("item", "top");
  XML::Element& sub = root.add("container");
  sub.add("item", "nested");
  XML::Element& deep = sub.add("deep");
  deep.add("item", "deep-nested");

  auto items = root.get_descendants("item");
  EXPECT_EQ(3, static_cast<int>(items.size()));
}

TEST(ElementDescendantTest, TestGetDescendantsWithPruning)
{
  XML::Element root("root");
  root.add("item", "top");
  XML::Element& container = root.add("container");
  container.add("item", "nested");

  auto items = root.get_descendants("item", "container");
  EXPECT_EQ(1, static_cast<int>(items.size()));
}

TEST(ElementDescendantTest, TestGetDescendantsSelfPrune)
{
  XML::Element root("root");
  root.add("item", "top");
  XML::Element& item2 = root.add("item", "nested-parent");
  item2.add("item", "deep");

  auto items = root.get_descendants("item", "item");
  EXPECT_EQ(2, static_cast<int>(items.size()));
}

//==========================================================================
// Attribute get tests
//==========================================================================

TEST(ElementAttrGetTest, TestGetAttr)
{
  XML::Element e("test");
  e.set_attr("key", "value");

  EXPECT_EQ("value", e.get_attr("key"));
  EXPECT_EQ("", e.get_attr("missing"));
  EXPECT_EQ("default", e.get_attr("missing", "default"));
}

TEST(ElementAttrGetTest, TestOperatorBracket)
{
  XML::Element e("test");
  e.set_attr("key", "value");

  EXPECT_EQ("value", e["key"]);
  EXPECT_EQ("", e["missing"]);

  const char *cstr = "key";
  EXPECT_EQ("value", e[cstr]);
}

TEST(ElementAttrGetTest, TestGetAttrBool)
{
  XML::Element e("test");
  e.set_attr("true1", "true");
  e.set_attr("true2", "True");
  e.set_attr("true3", "yes");
  e.set_attr("true4", "Yes");
  e.set_attr("true5", "1");
  e.set_attr("false1", "false");
  e.set_attr("false2", "no");
  e.set_attr("false3", "0");
  e.set_attr("empty", "");

  EXPECT_TRUE(e.get_attr_bool("true1"));
  EXPECT_TRUE(e.get_attr_bool("true2"));
  EXPECT_TRUE(e.get_attr_bool("true3"));
  EXPECT_TRUE(e.get_attr_bool("true4"));
  EXPECT_TRUE(e.get_attr_bool("true5"));
  EXPECT_FALSE(e.get_attr_bool("false1"));
  EXPECT_FALSE(e.get_attr_bool("false2"));
  EXPECT_FALSE(e.get_attr_bool("false3"));
  EXPECT_FALSE(e.get_attr_bool("empty"));
  EXPECT_FALSE(e.get_attr_bool("missing"));
  EXPECT_TRUE(e.get_attr_bool("missing", true));
}

TEST(ElementAttrGetTest, TestGetAttrInt)
{
  XML::Element e("test");
  e.set_attr("num", "42");
  e.set_attr("neg", "-7");
  e.set_attr("zero", "0");
  e.set_attr("bogus", "abc");

  EXPECT_EQ(42, e.get_attr_int("num"));
  EXPECT_EQ(-7, e.get_attr_int("neg"));
  EXPECT_EQ(0, e.get_attr_int("zero"));
  EXPECT_EQ(0, e.get_attr_int("bogus"));
  EXPECT_EQ(0, e.get_attr_int("missing"));
  EXPECT_EQ(99, e.get_attr_int("missing", 99));
}

TEST(ElementAttrGetTest, TestGetAttrHex)
{
  XML::Element e("test");
  e.set_attr("hex", "ff");
  e.set_attr("hex2", "1a");

  EXPECT_EQ(255, e.get_attr_hex("hex"));
  EXPECT_EQ(26, e.get_attr_hex("hex2"));
  EXPECT_EQ(0, e.get_attr_hex("missing"));
  EXPECT_EQ(16, e.get_attr_hex("missing", 16));
}

TEST(ElementAttrGetTest, TestGetAttrInt64)
{
  XML::Element e("test");
  e.set_attr("big", "5000000000");

  EXPECT_EQ(5000000000ULL, e.get_attr_int64("big"));
  EXPECT_EQ(0ULL, e.get_attr_int64("missing"));
  EXPECT_EQ(42ULL, e.get_attr_int64("missing", 42));
}

TEST(ElementAttrGetTest, TestGetAttrHex64)
{
  XML::Element e("test");
  e.set_attr("hex64", "ffffffff00");

  EXPECT_EQ(0xffffffff00ULL, e.get_attr_hex64("hex64"));
  EXPECT_EQ(0ULL, e.get_attr_hex64("missing"));
}

TEST(ElementAttrGetTest, TestGetAttrReal)
{
  XML::Element e("test");
  e.set_attr("pi", "3.14159");
  e.set_attr("neg", "-1.5");

  EXPECT_DOUBLE_EQ(3.14159, e.get_attr_real("pi"));
  EXPECT_DOUBLE_EQ(-1.5, e.get_attr_real("neg"));
  EXPECT_DOUBLE_EQ(0.0, e.get_attr_real("missing"));
  EXPECT_DOUBLE_EQ(2.5, e.get_attr_real("missing", 2.5));
}

TEST(ElementAttrGetTest, TestHasAttr)
{
  XML::Element e("test");
  e.set_attr("exists", "value");
  e.set_attr("empty", "");

  EXPECT_TRUE(e.has_attr("exists"));
  EXPECT_TRUE(e.has_attr("empty"));
  EXPECT_FALSE(e.has_attr("missing"));
}

TEST(ElementAttrGetTest, TestGetPrefixedAttributes)
{
  XML::Element e("foo");
  e.set_attr("prefix-x", "X");
  e.set_attr("prefix-y", "Y");
  e.set_attr("ignore", "bar");

  auto attrs = e.get_attrs_with_prefix("prefix-");
  ASSERT_EQ(2, static_cast<int>(attrs.size()));
  EXPECT_EQ("X", attrs["x"]);
  EXPECT_EQ("Y", attrs["y"]);
}

//==========================================================================
// Attribute set tests
//==========================================================================

TEST(ElementAttrSetTest, TestSetAttrChaining)
{
  XML::Element e("test");
  e.set_attr("a", "1").set_attr("b", "2").set_attr("c", "3");

  EXPECT_EQ("1", e["a"]);
  EXPECT_EQ("2", e["b"]);
  EXPECT_EQ("3", e["c"]);
}

TEST(ElementAttrSetTest, TestSetAttrInt)
{
  XML::Element e("test");
  e.set_attr_int("num", 42);
  e.set_attr_int("neg", -7);

  EXPECT_EQ("42", e["num"]);
  EXPECT_EQ("-7", e["neg"]);
  EXPECT_EQ(42, e.get_attr_int("num"));
}

TEST(ElementAttrSetTest, TestSetAttrHex)
{
  XML::Element e("test");
  e.set_attr_hex("hex", 255);

  EXPECT_EQ("ff", e["hex"]);
  EXPECT_EQ(255, e.get_attr_hex("hex"));
}

TEST(ElementAttrSetTest, TestSetAttrInt64)
{
  XML::Element e("test");
  e.set_attr_int64("big", 5000000000ULL);

  EXPECT_EQ("5000000000", e["big"]);
  EXPECT_EQ(5000000000ULL, e.get_attr_int64("big"));
}

TEST(ElementAttrSetTest, TestSetAttrHex64)
{
  XML::Element e("test");
  e.set_attr_hex64("hex64", 0xffffffff00ULL);

  EXPECT_EQ("ffffffff00", e["hex64"]);
  EXPECT_EQ(0xffffffff00ULL, e.get_attr_hex64("hex64"));
}

TEST(ElementAttrSetTest, TestSetAttrBool)
{
  XML::Element e("test");
  e.set_attr_bool("yes", true);
  e.set_attr_bool("no", false);

  EXPECT_EQ("true", e["yes"]);
  EXPECT_EQ("false", e["no"]);
  EXPECT_TRUE(e.get_attr_bool("yes"));
  EXPECT_FALSE(e.get_attr_bool("no"));
}

TEST(ElementAttrSetTest, TestSetAttrReal)
{
  XML::Element e("test");
  e.set_attr_real("pi", 3.14159);

  EXPECT_DOUBLE_EQ(3.14159, e.get_attr_real("pi"));
}

TEST(ElementAttrSetTest, TestRemoveAttr)
{
  XML::Element e("test");
  e.set_attr("key", "value");
  ASSERT_TRUE(e.has_attr("key"));

  e.remove_attr("key");
  EXPECT_FALSE(e.has_attr("key"));
}

TEST(ElementAttrSetTest, TestRemoveAttrChaining)
{
  XML::Element e("test");
  e.set_attr("a", "1").set_attr("b", "2");
  e.remove_attr("a").set_attr("c", "3");

  EXPECT_FALSE(e.has_attr("a"));
  EXPECT_TRUE(e.has_attr("b"));
  EXPECT_TRUE(e.has_attr("c"));
}

//==========================================================================
// Content tests
//==========================================================================

TEST(ElementContentTest, TestGetContentOptimised)
{
  XML::Element e("foo", "hello");
  EXPECT_EQ("hello", e.get_content());
  EXPECT_EQ("hello", *e);
}

TEST(ElementContentTest, TestGetContentFromTextChildren)
{
  XML::Element root("root");
  XML::Element *text1 = new XML::Element();
  text1->content = "hello";
  root.add(text1);
  XML::Element *text2 = new XML::Element();
  text2->content = "world";
  root.add(text2);

  string content = root.get_content();
  EXPECT_EQ("hello\nworld\n", content);
}

TEST(ElementContentTest, TestGetDeepContent)
{
  XML::Element root("root");
  root.add("child", "nested");

  EXPECT_EQ("nested\n", root.get_deep_content());
}

TEST(ElementContentTest, TestGetDeepContentOptimised)
{
  XML::Element root("root", "direct");
  EXPECT_EQ("direct", root.get_deep_content());
}

//==========================================================================
// Add child tests
//==========================================================================

TEST(ElementAddTest, TestAddByPointer)
{
  XML::Element root("root");
  auto *child = new XML::Element("child");
  root.add(child);

  EXPECT_EQ(1, static_cast<int>(root.children.size()));
  EXPECT_EQ(&root, child->parent);
}

TEST(ElementAddTest, TestAddByReference)
{
  XML::Element root("root");
  XML::Element child("child");
  child.set_attr("id", "1");

  root.add(child);
  EXPECT_EQ(1, static_cast<int>(root.children.size()));
  EXPECT_EQ("1", root.get_child("child").get_attr("id"));
}

TEST(ElementAddTest, TestAddByName)
{
  XML::Element root("root");
  XML::Element& child = root.add("child");

  EXPECT_EQ("child", child.name);
  EXPECT_EQ(&root, child.parent);
}

TEST(ElementAddTest, TestAddNameContent)
{
  XML::Element root("root");
  root.add("child", "data");

  EXPECT_EQ("data", root.get_child("child").content);
}

TEST(ElementAddTest, TestAddNameAttrValue)
{
  XML::Element root("root");
  root.add("child", "id", "42");

  EXPECT_EQ("42", root.get_child("child").get_attr("id"));
}

TEST(ElementAddTest, TestAddNameAttrValueContent)
{
  XML::Element root("root");
  root.add("child", "id", "42", "data");

  XML::Element& child = root.get_child("child");
  EXPECT_EQ("42", child.get_attr("id"));
  EXPECT_EQ("data", child.content);
}

TEST(ElementAddTest, TestAddXml)
{
  XML::Element root("root");
  ostringstream err;
  XML::Element& added = root.add_xml("<child id='1'>text</child>", err);

  ASSERT_TRUE(added.valid());
  EXPECT_EQ("child", added.name);
  EXPECT_EQ("text", added.content);
  EXPECT_EQ("1", added.get_attr("id"));
}

TEST(ElementAddTest, TestAddXmlInvalid)
{
  XML::Element root("root");
  ostringstream err;
  XML::Element& added = root.add_xml("<invalid", err);

  EXPECT_FALSE(added.valid());
}

//==========================================================================
// Merge tests
//==========================================================================

TEST(ElementMergeTest, TestMerge)
{
  XML::Element dest("root");
  dest.set_attr("a", "1");
  dest.add("child1");

  XML::Element source("root");
  source.set_attr("a", "2");
  source.set_attr("b", "3");
  source.add("child2");

  dest.merge(source);

  EXPECT_EQ("2", dest.get_attr("a"));
  EXPECT_EQ("3", dest.get_attr("b"));
  EXPECT_EQ(2, static_cast<int>(dest.children.size()));
  EXPECT_TRUE(dest.get_child("child1").valid());
  EXPECT_TRUE(dest.get_child("child2").valid());
}

TEST(ElementMergeTest, TestMergeXml)
{
  XML::Element root("foo");
  root.set_attr("a", "1");

  ostringstream err;
  EXPECT_TRUE(root.merge_xml("<foo b='2'><child/></foo>", err));

  EXPECT_EQ("1", root.get_attr("a"));
  EXPECT_EQ("2", root.get_attr("b"));
  EXPECT_TRUE(root.get_child("child").valid());
}

TEST(ElementMergeTest, TestMergeXmlWrongName)
{
  XML::Element root("foo");
  ostringstream err;
  EXPECT_FALSE(root.merge_xml("<bar/>", err));
}

TEST(ElementMergeTest, TestMergeXmlInvalid)
{
  XML::Element root("foo");
  ostringstream err;
  EXPECT_FALSE(root.merge_xml("<invalid", err));
}

//==========================================================================
// Superimpose tests
//==========================================================================

TEST(ElementSuperimposeTest, TestSuperimpose)
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

TEST(ElementSuperimposeTest, TestSuperimposeByName)
{
  XML::Element a("root");
  a.add("alpha", "original");

  XML::Element b("root");
  b.add("alpha", "replaced");
  b.add("beta", "new");

  a.superimpose(b);

  EXPECT_EQ("replaced", a.get_child("alpha").content);
  EXPECT_TRUE(a.get_child("beta").valid());
}

TEST(ElementSuperimposeTest, TestSuperimposeContentOverride)
{
  XML::Element a("root", "original");
  XML::Element b("root", "replaced");
  a.superimpose(b);
  EXPECT_EQ("replaced", a.content);
}

//==========================================================================
// Serialization tests
//==========================================================================

TEST(ElementSerializationTest, TestToStringEmpty)
{
  XML::Element e("empty");
  EXPECT_EQ("<empty/>\n", e.to_string());
}

TEST(ElementSerializationTest, TestToStringWithContent)
{
  XML::Element e("foo", "bar");
  EXPECT_EQ("<foo>bar</foo>\n", e.to_string());
}

TEST(ElementSerializationTest, TestToStringWithChildren)
{
  XML::Element e("root");
  e.add("child");

  EXPECT_EQ("<root>\n  <child/>\n</root>\n", e.to_string());
}

TEST(ElementSerializationTest, TestToStringWithPI)
{
  XML::Element e("root");
  string result = e.to_string(true);
  EXPECT_EQ("<?xml version=\"1.0\"?>\n<root/>\n", result);
}

TEST(ElementSerializationTest, TestStartToString)
{
  XML::Element e("foo");
  e.set_attr("id", "1");
  EXPECT_EQ("<foo id=\"1\">", e.start_to_string());
}

TEST(ElementSerializationTest, TestEndToString)
{
  XML::Element e("foo");
  EXPECT_EQ("</foo>", e.end_to_string());
}

TEST(ElementSerializationTest, TestStreamOperator)
{
  XML::Element e("test");
  ostringstream oss;
  oss << e;
  EXPECT_EQ("<test/>\n", oss.str());
}

TEST(ElementSerializationTest, TestAttributeEscaping)
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
  ASSERT_EQ("<test amp=\"&amp;foo\" bothquot=\"'&quot;foo\" dquot='\"foo'"
            " gt=\"&gt;foo\" lt=\"&lt;foo\" normal=\"foo\" squot=\"'foo\"/>\n",
            streamed_xml);
}

TEST(ElementSerializationTest, TestContentEscaping)
{
  XML::Element e("test", "a<b>c&d");
  string s = e.to_string();
  EXPECT_EQ("<test>a&lt;b&gt;c&amp;d</test>\n", s);
}

//==========================================================================
// Tree operation tests
//==========================================================================

TEST(ElementTreeTest, TestOptimise)
{
  XML::Element root("root");
  auto *text = new XML::Element();
  text->content = "hello";
  root.add(text);

  EXPECT_EQ(1, static_cast<int>(root.children.size()));
  root.optimise();
  EXPECT_EQ(0, static_cast<int>(root.children.size()));
  EXPECT_EQ("hello", root.content);
}

TEST(ElementTreeTest, TestOptimiseMultipleChildrenNoOp)
{
  XML::Element root("root");
  root.add("child1");
  root.add("child2");
  root.optimise();
  EXPECT_EQ(2, static_cast<int>(root.children.size()));
}

TEST(ElementTreeTest, TestTranslateRename)
{
  XML::Element root("root");
  root.add("old_name", "data");

  map<string, string> trans;
  trans["old_name"] = "new_name";

  root.translate(trans);
  EXPECT_FALSE(root.get_child("old_name").valid());
  EXPECT_TRUE(root.get_child("new_name").valid());
  EXPECT_EQ("data", root.get_child("new_name").content);
}

TEST(ElementTreeTest, TestTranslateDelete)
{
  XML::Element root("root");
  root.add("keep_me");
  root.add("delete_me");

  map<string, string> trans;
  trans["delete_me"] = "";

  root.translate(trans);
  EXPECT_TRUE(root.get_child("keep_me").valid());
  EXPECT_FALSE(root.get_child("delete_me").valid());
  EXPECT_EQ(1, static_cast<int>(root.children.size()));
}

TEST(ElementTreeTest, TestTranslateLeaveUnmapped)
{
  XML::Element root("root");
  root.add("unmapped");

  map<string, string> trans;
  trans["other"] = "changed";

  root.translate(trans);
  EXPECT_TRUE(root.get_child("unmapped").valid());
}

TEST(ElementTreeTest, TestDetach)
{
  XML::Element root("root");
  XML::Element& child = root.add("child");

  EXPECT_EQ(1, static_cast<int>(root.children.size()));
  child.detach();
  EXPECT_EQ(0, static_cast<int>(root.children.size()));
  EXPECT_EQ(nullptr, child.parent);

  delete &child;
}

TEST(ElementTreeTest, TestReplaceWith)
{
  XML::Element root("root");
  root.add("first");
  XML::Element& mid = root.add("middle");
  root.add("last");

  auto *replacement = new XML::Element("replaced");
  mid.replace_with(replacement);

  EXPECT_EQ(3, static_cast<int>(root.children.size()));
  auto it = root.children.begin();
  EXPECT_EQ("first", (*it)->name);
  ++it;
  EXPECT_EQ("replaced", (*it)->name);
  ++it;
  EXPECT_EQ("last", (*it)->name);

  delete &mid;
}

TEST(ElementTreeTest, TestRemoveChildren)
{
  XML::Element root("root");
  root.add("keep");
  root.add("remove");
  root.add("keep");
  root.add("remove");

  root.remove_children("remove");
  EXPECT_EQ(2, static_cast<int>(root.children.size()));
  EXPECT_FALSE(root.get_child("remove").valid());
}

TEST(ElementTreeTest, TestClearChildren)
{
  XML::Element root("root");
  root.add("a");
  root.add("b");
  root.add("c");

  root.clear_children();
  EXPECT_TRUE(root.children.empty());
}

TEST(ElementTreeTest, TestAddPrefix)
{
  XML::Element a("root");
  a.add("child");
  a.add("foo:child");

  a.add_prefix("foo:");

  ASSERT_EQ("foo:root", a.name);
  auto it = a.children.begin();
  EXPECT_EQ("foo:child", (*it)->name);
  ++it;
  EXPECT_EQ("foo:child", (*it)->name);
}

TEST(ElementTreeTest, TestRemovePrefix)
{
  XML::Element a("foo:root");
  a.add("foo:child");
  a.add("bar:child");
  a.add("foo:");

  a.remove_prefix("foo:");

  ASSERT_EQ("root", a.name);
  auto it = a.children.begin();
  EXPECT_EQ("child", (*it)->name);
  ++it;
  EXPECT_EQ("bar:child", (*it)->name);
  ++it;
  EXPECT_EQ("", (*it)->name);
}

//==========================================================================
// get_xpath tests
//==========================================================================

TEST(ElementXPathPositionTest, TestGetXpathSimple)
{
  XML::Element root("root");
  XML::Element& child = root.add("child");

  EXPECT_EQ("/child", child.get_xpath());
}

TEST(ElementXPathPositionTest, TestGetXpathNested)
{
  XML::Element root("root");
  XML::Element& mid = root.add("middle");
  XML::Element& leaf = mid.add("leaf");

  EXPECT_EQ("/middle/leaf", leaf.get_xpath());
}

TEST(ElementXPathPositionTest, TestGetXpathIndexed)
{
  XML::Element root("root");
  root.add("item");
  XML::Element& second = root.add("item");

  EXPECT_EQ("/item[2]", second.get_xpath());
}

TEST(ElementXPathPositionTest, TestGetXpathRoot)
{
  XML::Element root("root");
  EXPECT_EQ("", root.get_xpath());
}

//==========================================================================
// ElementIterator tests
//==========================================================================

TEST(ElementIteratorTest, TestBasicIteration)
{
  XML::Element root("root");
  root.add("a");
  root.add("b");
  root.add("c");

  vector<string> names;
  for (XML::Element::iterator it(root.children); it; ++it)
    names.push_back((*it).name);

  ASSERT_EQ(3, static_cast<int>(names.size()));
  EXPECT_EQ("a", names[0]);
  EXPECT_EQ("b", names[1]);
  EXPECT_EQ("c", names[2]);
}

TEST(ElementIteratorTest, TestEmptyIteration)
{
  XML::Element root("root");
  XML::Element::iterator it(root.children);
  EXPECT_FALSE(it.valid());
  EXPECT_FALSE(static_cast<bool>(it));
  EXPECT_TRUE(!it);
}

TEST(ElementIteratorTest, TestCopyConstructor)
{
  XML::Element root("root");
  root.add("a");
  root.add("b");

  XML::Element::iterator original(root.children);
  XML::Element::iterator copy(original);

  EXPECT_TRUE(copy.valid());
  EXPECT_EQ("a", (*copy).name);
}

TEST(ElementIteratorTest, TestArrowOperator)
{
  XML::Element root("root");
  root.add("child", "data");

  XML::Element::iterator it(root.children);
  EXPECT_EQ("child", it->name);
  EXPECT_EQ("data", it->content);
}

TEST(ConstElementIteratorTest, TestBasicIteration)
{
  XML::Element root("root");
  root.add("a");
  root.add("b");

  auto children = root.get_children();
  XML::ConstElementIterator it(children);

  EXPECT_TRUE(it.valid());
  EXPECT_EQ("a", (*it).name);
  ++it;
  EXPECT_EQ("b", (*it).name);
  ++it;
  EXPECT_FALSE(it.valid());
}

//==========================================================================
// Parser tests
//==========================================================================

TEST(ParserTest, TestBasicParse)
{
  string xml = "<foo><bar/></foo>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ(1, static_cast<int>(e.children.size()));
  EXPECT_FALSE(!e.get_child("bar"));
}

TEST(ParserTest, TestContentOptimisation)
{
  string xml = "<foo>content</foo>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("content", *e);
}

TEST(ParserTest, TestContentOptimisationWithComment)
{
  string xml = "<foo>content<!--comment-->more</foo>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_FALSE(!e);
  EXPECT_EQ("foo", e.name);
  EXPECT_EQ("content more", *e);
}

TEST(ParserTest, TestParseAttributes)
{
  string xml = "<foo id=\"1\" name='test' flag=\"true\"/>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  ASSERT_TRUE(e.valid());
  EXPECT_EQ("1", e["id"]);
  EXPECT_EQ("test", e["name"]);
  EXPECT_EQ("true", e["flag"]);
}

TEST(ParserTest, TestParseNestedElements)
{
  string xml = "<root><a><b><c>deep</c></b></a></root>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& root = parser.get_root();
  EXPECT_EQ("deep", root.get_child("a").get_child("b").get_child("c").content);
}

TEST(ParserTest, TestParseSiblings)
{
  string xml = "<root><a/><b/><c/></root>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& root = parser.get_root();

  EXPECT_EQ(3, static_cast<int>(root.children.size()));
  EXPECT_TRUE(root.get_child("a").valid());
  EXPECT_TRUE(root.get_child("b").valid());
  EXPECT_TRUE(root.get_child("c").valid());
}

TEST(ParserTest, TestParseEmptyElement)
{
  string xml = "<empty/>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  EXPECT_EQ("empty", e.name);
  EXPECT_TRUE(e.children.empty());
  EXPECT_EQ("", e.content);
}

TEST(ParserTest, TestParseEntityReferences)
{
  string xml = "<test attr=\"&lt;&gt;&amp;&quot;\">&lt;&gt;&amp;</test>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();

  EXPECT_EQ("<>&\"", e["attr"]);
  EXPECT_EQ("<>&", *e);
}

TEST(ParserTest, TestParseNumericCharRef)
{
  string xml = "<test>&#42;&#x2a;</test>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  EXPECT_EQ("**", *e);
}

TEST(ParserTest, TestParseAposEntity)
{
  string xml = "<test attr='&apos;hello&apos;'/>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  EXPECT_EQ("'hello'", e["attr"]);
}

TEST(ParserTest, TestParseComment)
{
  string xml = "<root><!-- this is a comment --><child/></root>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& root = parser.get_root();
  EXPECT_TRUE(root.get_child("child").valid());
}

TEST(ParserTest, TestParseProcessingInstruction)
{
  string xml = "<?xml version=\"1.0\"?><root/>";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
  EXPECT_TRUE(parser.get_root().valid());
}

TEST(ParserTest, TestPreserveWhitespace)
{
  string xml = "<root>  text  </root>";
  XML::Parser parser(XML::PARSER_PRESERVE_WHITESPACE);
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  EXPECT_EQ(1, static_cast<int>(e.children.size()));
}

TEST(ParserTest, TestNoContentOptimisation)
{
  string xml = "<foo>content</foo>";
  XML::Parser parser(0);
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  EXPECT_EQ("", e.content);
  EXPECT_EQ(1, static_cast<int>(e.children.size()));
}

TEST(ParserTest, TestParseFromStream)
{
  string xml = "<root><child/></root>";
  istringstream iss(xml);
  XML::Parser parser;
  ASSERT_NO_THROW(iss >> parser);
  EXPECT_EQ("root", parser.get_root().name);
}

TEST(ParserTest, TestParseInvalidXml)
{
  string xml = "<unclosed>";
  XML::Parser parser;
  EXPECT_THROW(parser.read_from(xml), XML::ParseFailed);
}

TEST(ParserTest, TestParseMismatchedTags)
{
  string xml = "<foo></bar>";
  ostringstream err;
  XML::Parser parser(err);
  EXPECT_THROW(parser.read_from(xml), XML::ParseFailed);
}

TEST(ParserTest, TestParseEmptyInput)
{
  string xml = "";
  XML::Parser parser;
  EXPECT_THROW(parser.read_from(xml), XML::ParseFailed);
}

TEST(ParserTest, TestIgnoreBOM)
{
  string xml = "\xef\xbb\xbf<foo/>\n";
  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));
}

TEST(ParserTest, TestFailOnBogusBOM)
{
  string xml = "\xef\xbf\xbb<foo/>\n";
  XML::Parser parser;
  ASSERT_THROW(parser.read_from(xml), XML::ParseFailed);
}

TEST(ParserTest, TestParserErrorCount)
{
  string xml = "<root><child/></root>";
  XML::Parser parser;
  parser.read_from(xml);
  EXPECT_EQ(0, parser.errors);
}

TEST(ParserTest, TestDetachRoot)
{
  string xml = "<root/>";
  XML::Parser parser;
  parser.read_from(xml);

  XML::Element *root = parser.detach_root();
  ASSERT_NE(nullptr, root);
  EXPECT_EQ("root", root->name);

  EXPECT_FALSE(parser.get_root().valid());
  delete root;
}

TEST(ParserTest, TestReplaceRoot)
{
  string xml = "<root/>";
  XML::Parser parser;
  parser.read_from(xml);

  auto *new_root = new XML::Element("new_root");
  parser.replace_root(new_root);
  EXPECT_EQ("new_root", parser.get_root().name);
}

TEST(ParserTest, TestNamespaceFixing)
{
  string xml = "<root xmlns:ns1=\"mynamespace\"><ns1:child/></root>";
  XML::Parser parser;
  parser.fix_namespace("mynamespace", "fixed");
  ASSERT_NO_THROW(parser.read_from(xml));

  XML::Element& root = parser.get_root();
  EXPECT_TRUE(root.get_child("fixed:child").valid());
}

TEST(ParserTest, TestParseMixedContent)
{
  string xml = "<root>text<child/>more</root>";
  XML::Parser parser(XML::PARSER_PRESERVE_WHITESPACE);
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& root = parser.get_root();
  EXPECT_EQ(3, static_cast<int>(root.children.size()));
}

TEST(ParserTest, TestMultipleParserRoundtrip)
{
  string xml = "<root attr=\"val\"><child>text</child></root>\n";
  XML::Parser parser;
  parser.read_from(xml);
  string output = parser.get_root().to_string();

  XML::Parser parser2;
  parser2.read_from(output);
  EXPECT_EQ("val", parser2.get_root()["attr"]);
  EXPECT_EQ("text", parser2.get_root().get_child("child").content);
}

TEST(ParserTest, TestParseComplexXml)
{
  string xml =
    "<?xml version=\"1.0\"?>\n"
    "<bookstore>\n"
    "  <book genre=\"autobiography\">\n"
    "    <title>The Autobiography of Benjamin Franklin</title>\n"
    "    <author>\n"
    "      <first-name>Benjamin</first-name>\n"
    "      <last-name>Franklin</last-name>\n"
    "    </author>\n"
    "    <price>8.99</price>\n"
    "  </book>\n"
    "  <book genre=\"novel\">\n"
    "    <title>The Confidence Man</title>\n"
    "    <author>\n"
    "      <first-name>Herman</first-name>\n"
    "      <last-name>Melville</last-name>\n"
    "    </author>\n"
    "    <price>11.99</price>\n"
    "  </book>\n"
    "</bookstore>\n";

  XML::Parser parser;
  ASSERT_NO_THROW(parser.read_from(xml));

  XML::Element& root = parser.get_root();
  EXPECT_EQ("bookstore", root.name);

  auto books = root.get_children("book");
  ASSERT_EQ(2, static_cast<int>(books.size()));

  auto it = books.begin();
  EXPECT_EQ("autobiography", (*it)->get_attr("genre"));
  EXPECT_EQ("The Autobiography of Benjamin Franklin",
            (*it)->get_child("title").content);
  ++it;
  EXPECT_EQ("novel", (*it)->get_attr("genre"));
}

TEST(ParserTest, TestLenientParsing)
{
  string xml = "<root>a && b</root>";
  XML::Parser parser(XML::PARSER_BE_LENIENT | XML::PARSER_OPTIMISE_CONTENT);
  ASSERT_NO_THROW(parser.read_from(xml));
  XML::Element& e = parser.get_root();
  string content = *e;
  EXPECT_NE(string::npos, content.find("&&"));
}

TEST(ParserTest, TestParseLineNumbers)
{
  string xml = "<root>\n<child/>\n</root>";
  XML::Parser parser;
  parser.read_from(xml);
  XML::Element& root = parser.get_root();
  EXPECT_EQ(1, root.line);
}

TEST(ParserTest, TestParseParentPointers)
{
  string xml = "<root><child><grandchild/></child></root>";
  XML::Parser parser;
  parser.read_from(xml);
  XML::Element& root = parser.get_root();

  XML::Element& child = root.get_child("child");
  EXPECT_EQ(&root, child.parent);

  XML::Element& grandchild = child.get_child("grandchild");
  EXPECT_EQ(&child, grandchild.parent);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
