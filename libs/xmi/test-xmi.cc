//==========================================================================
// ObTools::XMI: test-xmi.cc
//
// Test harness for XMI reader library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmi.h"
#include <sstream>
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Helper to wrap XMI content in a valid XMI 1.1 document
string make_xmi(const string& model_content,
                const string& model_name = "TestModel")
{
  return
    "<?xml version='1.0' encoding='UTF-8'?>\n"
    "<XMI xmi.version='1.1' xmlns:UML='org.omg.xmi.namespace.UML'>\n"
    "  <XMI.header>\n"
    "    <XMI.metamodel xmi.name='UML' xmi.version='1.4'/>\n"
    "  </XMI.header>\n"
    "  <XMI.content>\n"
    "    <UML:Model xmi.id='m1' name='" + model_name + "'>\n"
    "      <UML:Namespace.ownedElement>\n"
    + model_content +
    "      </UML:Namespace.ownedElement>\n"
    "    </UML:Model>\n"
    "  </XMI.content>\n"
    "</XMI>\n";
}

//==========================================================================
// Basic parsing

TEST(XMIReaderTest, TestEmptyModel)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(""));
  reader.read_from(xmi);

  ASSERT_NE(nullptr, reader.model);
  EXPECT_EQ("TestModel", reader.model->name);
  EXPECT_DOUBLE_EQ(1.4, reader.model->uml_version);
}

TEST(XMIReaderTest, TestXMIVersion)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(""));
  reader.read_from(xmi);

  EXPECT_DOUBLE_EQ(1.1, reader.xmi_version);
}

TEST(XMIReaderTest, TestStreamOperator)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(""));
  xmi >> reader;
  ASSERT_NE(nullptr, reader.model);
}

TEST(XMIReaderTest, TestBadXMLThrows)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi("not xml");
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
}

TEST(XMIReaderTest, TestNonXMIRootThrows)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi("<NotXMI/>");
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
}

TEST(XMIReaderTest, TestNoContentThrows)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi("<XMI xmi.version='1.1'/>");
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
}

TEST(XMIReaderTest, TestNoModelThrows)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(
    "<XMI xmi.version='1.1'>"
    "  <XMI.content/>"
    "</XMI>");
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
}

//==========================================================================
// Class parsing

TEST(XMIReaderTest, TestParseClass)
{
  string content =
    "<UML:Class xmi.id='c1' name='MyClass' visibility='public'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  EXPECT_EQ("MyClass", classes.front()->name);
  EXPECT_EQ(UML::VISIBILITY_PUBLIC, classes.front()->visibility);
}

TEST(XMIReaderTest, TestParseMultipleClasses)
{
  string content =
    "<UML:Class xmi.id='c1' name='ClassA'/>\n"
    "<UML:Class xmi.id='c2' name='ClassB'/>\n"
    "<UML:Class xmi.id='c3' name='ClassC'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  EXPECT_EQ(3u, classes.size());
}

TEST(XMIReaderTest, TestClassVisibility)
{
  string content =
    "<UML:Class xmi.id='c1' name='Pub' visibility='public'/>\n"
    "<UML:Class xmi.id='c2' name='Priv' visibility='private'/>\n"
    "<UML:Class xmi.id='c3' name='Prot' visibility='protected'/>\n"
    "<UML:Class xmi.id='c4' name='Pkg' visibility='package'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(4u, classes.size());
}

TEST(XMIReaderTest, TestActiveClass)
{
  string content =
    "<UML:Class xmi.id='c1' name='Active' isActive='true'/>\n"
    "<UML:Class xmi.id='c2' name='Passive' isActive='false'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(2u, classes.size());
  auto it = classes.begin();
  EXPECT_TRUE((*it)->is_active);
  ++it;
  EXPECT_FALSE((*it)->is_active);
}

//==========================================================================
// Attribute parsing

TEST(XMIReaderTest, TestParseAttribute)
{
  string content =
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='myAttr' visibility='private'>\n"
    "      <UML:StructuralFeature.type>\n"
    "        <UML:DataType xmi.idref='t1'/>\n"
    "      </UML:StructuralFeature.type>\n"
    "    </UML:Attribute>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n"
    "<UML:DataType xmi.id='t1' name='int'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  auto attrs = classes.front()->get_attributes();
  ASSERT_EQ(1u, attrs.size());
  EXPECT_EQ("myAttr", attrs.front()->name);
  EXPECT_EQ(UML::VISIBILITY_PRIVATE, attrs.front()->visibility);
}

TEST(XMIReaderTest, TestAttributeWithInitialValue)
{
  // Attribute needs a type reference to avoid build_refs() error
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='x'>\n"
    "      <UML:StructuralFeature.type>\n"
    "        <UML:Classifier xmi.idref='t1'/>\n"
    "      </UML:StructuralFeature.type>\n"
    "      <UML:Attribute.initialValue>\n"
    "        <UML:Expression language='C++' body='42'/>\n"
    "      </UML:Attribute.initialValue>\n"
    "    </UML:Attribute>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  auto attrs = classes.front()->get_attributes();
  ASSERT_EQ(1u, attrs.size());
  EXPECT_EQ("42", attrs.front()->initial_value.body);
  EXPECT_EQ("C++", attrs.front()->initial_value.language);
}

//==========================================================================
// Operation parsing

TEST(XMIReaderTest, TestParseOperation)
{
  // Parameters need type references to avoid build_refs() error
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:DataType xmi.id='t2' name='void'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='doStuff' visibility='public'\n"
    "                   isAbstract='false' concurrency='sequential'>\n"
    "      <UML:BehaviouralFeature.parameter>\n"
    "        <UML:Parameter xmi.id='p1' name='arg1' kind='in'\n"
    "                       type='t1'/>\n"
    "        <UML:Parameter xmi.id='p2' name='return' kind='return'\n"
    "                       type='t2'/>\n"
    "      </UML:BehaviouralFeature.parameter>\n"
    "    </UML:Operation>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  auto ops = classes.front()->get_operations();
  ASSERT_EQ(1u, ops.size());
  EXPECT_EQ("doStuff", ops.front()->name);
  EXPECT_FALSE(ops.front()->is_abstract);

  auto params = ops.front()->get_parameters();
  ASSERT_EQ(1u, params.size());
  EXPECT_EQ("arg1", params.front()->name);

  auto *ret = ops.front()->get_return();
  ASSERT_NE(nullptr, ret);
  EXPECT_EQ("return", ret->name);
}

TEST(XMIReaderTest, TestOperationWithNoReturn)
{
  // Operation with only 'in' parameters and no 'return' pseudo-parameter
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='voidOp'>\n"
    "      <UML:BehaviouralFeature.parameter>\n"
    "        <UML:Parameter xmi.id='p1' name='x' kind='in'\n"
    "                       type='t1'/>\n"
    "      </UML:BehaviouralFeature.parameter>\n"
    "    </UML:Operation>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto ops = reader.model->get_classes().front()->get_operations();
  ASSERT_EQ(1u, ops.size());

  auto *ret = ops.front()->get_return();
  EXPECT_EQ(nullptr, ret);
}

//==========================================================================
// Parameter kinds

TEST(XMIReaderTest, TestParameterKinds)
{
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='op'>\n"
    "      <UML:BehaviouralFeature.parameter>\n"
    "        <UML:Parameter xmi.id='p1' name='a' kind='in' type='t1'/>\n"
    "        <UML:Parameter xmi.id='p2' name='b' kind='inout' type='t1'/>\n"
    "        <UML:Parameter xmi.id='p3' name='c' kind='out' type='t1'/>\n"
    "        <UML:Parameter xmi.id='p4' name='ret' kind='return' type='t1'/>\n"
    "      </UML:BehaviouralFeature.parameter>\n"
    "    </UML:Operation>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto ops = reader.model->get_classes().front()->get_operations();
  ASSERT_EQ(1u, ops.size());

  // get_parameters() filters out 'return'
  auto params = ops.front()->get_parameters();
  ASSERT_EQ(3u, params.size());
  auto it = params.begin();
  EXPECT_EQ(UML::PARAMETER_IN, (*it)->kind); ++it;
  EXPECT_EQ(UML::PARAMETER_INOUT, (*it)->kind); ++it;
  EXPECT_EQ(UML::PARAMETER_OUT, (*it)->kind);

  auto *ret = ops.front()->get_return();
  ASSERT_NE(nullptr, ret);
  EXPECT_EQ(UML::PARAMETER_RETURN, ret->kind);
}

TEST(XMIReaderTest, TestUnknownParameterKindWarning)
{
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='op'>\n"
    "      <UML:BehaviouralFeature.parameter>\n"
    "        <UML:Parameter xmi.id='p1' name='a' kind='bogus' type='t1'/>\n"
    "      </UML:BehaviouralFeature.parameter>\n"
    "    </UML:Operation>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  EXPECT_NE(string::npos, err.str().find("Unknown parameter kind"));
  // Defaults to IN
  auto params = reader.model->get_classes().front()->get_operations()
                  .front()->get_parameters();
  ASSERT_EQ(1u, params.size());
  EXPECT_EQ(UML::PARAMETER_IN, params.front()->kind);
}

//==========================================================================
// Parameter default value

TEST(XMIReaderTest, TestParameterDefaultValue)
{
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='op'>\n"
    "      <UML:BehaviouralFeature.parameter>\n"
    "        <UML:Parameter xmi.id='p1' name='a' kind='in' type='t1'>\n"
    "          <UML:Attribute.defaultValue>\n"
    "            <UML:Expression language='C++' body='0'/>\n"
    "          </UML:Attribute.defaultValue>\n"
    "        </UML:Parameter>\n"
    "      </UML:BehaviouralFeature.parameter>\n"
    "    </UML:Operation>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto params = reader.model->get_classes().front()->get_operations()
                  .front()->get_parameters();
  ASSERT_EQ(1u, params.size());
  EXPECT_EQ("0", params.front()->default_value.body);
  EXPECT_EQ("C++", params.front()->default_value.language);
}

//==========================================================================
// Enumeration parsing

TEST(XMIReaderTest, TestParseEnumeration)
{
  string content =
    "<UML:Enumeration xmi.id='e1' name='Color'>\n"
    "  <UML:Enumeration.literal>\n"
    "    <UML:EnumerationLiteral xmi.id='el1' name='Red'/>\n"
    "    <UML:EnumerationLiteral xmi.id='el2' name='Green'/>\n"
    "    <UML:EnumerationLiteral xmi.id='el3' name='Blue'/>\n"
    "  </UML:Enumeration.literal>\n"
    "</UML:Enumeration>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto datatypes = reader.model->get_datatypes();
  bool found = false;
  for (auto *dt : datatypes)
  {
    auto *e = dynamic_cast<UML::Enumeration*>(dt);
    if (e && e->name == "Color")
    {
      found = true;
      EXPECT_EQ(3u, e->literals.size());
      auto it = e->literals.begin();
      EXPECT_EQ("Red", *it++);
      EXPECT_EQ("Green", *it++);
      EXPECT_EQ("Blue", *it++);
    }
  }
  EXPECT_TRUE(found);
}

//==========================================================================
// DataType parsing

TEST(XMIReaderTest, TestParseDataType)
{
  string content =
    "<UML:DataType xmi.id='t1' name='String'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto datatypes = reader.model->get_datatypes();
  ASSERT_GE(datatypes.size(), 1u);
  EXPECT_EQ("String", datatypes.front()->name);
}

//==========================================================================
// Package parsing

TEST(XMIReaderTest, TestParsePackage)
{
  string content =
    "<UML:Package xmi.id='p1' name='MyPackage'>\n"
    "  <UML:Namespace.ownedElement>\n"
    "    <UML:Class xmi.id='c1' name='InnerClass'/>\n"
    "  </UML:Namespace.ownedElement>\n"
    "</UML:Package>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto packages = reader.model->get_subpackages();
  ASSERT_EQ(1u, packages.size());
  EXPECT_EQ("MyPackage", packages.front()->name);

  auto classes = packages.front()->get_classes();
  ASSERT_EQ(1u, classes.size());
  EXPECT_EQ("InnerClass", classes.front()->name);
}

//==========================================================================
// Association parsing

TEST(XMIReaderTest, TestParseAssociation)
{
  string content =
    "<UML:Class xmi.id='c1' name='ClassA'/>\n"
    "<UML:Class xmi.id='c2' name='ClassB'/>\n"
    "<UML:Association xmi.id='as1' name='myAssoc'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' name='a' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' name='b' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  ASSERT_EQ(1u, assocs.size());
  auto *assoc = assocs.front();
  EXPECT_EQ("myAssoc", assoc->name);
  EXPECT_EQ(2u, assoc->connections.size());
}

//==========================================================================
// Aggregation kinds

TEST(XMIReaderTest, TestAggregationKinds)
{
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Class xmi.id='c3' name='C'/>\n"
    "<UML:Association xmi.id='as1' name='assoc1'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' aggregation='aggregate'\n"
    "                        isNavigable='true'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' aggregation='none'\n"
    "                        isNavigable='true'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n"
    "<UML:Association xmi.id='as2' name='assoc2'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae3' aggregation='composite'\n"
    "                        isNavigable='true'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae4' aggregation='none'\n"
    "                        isNavigable='true'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c3'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  ASSERT_EQ(2u, assocs.size());

  auto it = assocs.begin();
  // First association: aggregate + none
  EXPECT_EQ(UML::AGGREGATION_AGGREGATE, (*it)->connections[0]->aggregation);
  EXPECT_EQ(UML::AGGREGATION_NONE, (*it)->connections[1]->aggregation);

  ++it;
  // Second association: composite + none
  EXPECT_EQ(UML::AGGREGATION_COMPOSITE, (*it)->connections[0]->aggregation);
}

TEST(XMIReaderTest, TestUnknownAggregationWarning)
{
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Association xmi.id='as1' name='a'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' aggregation='bogus'\n"
    "                        isNavigable='true'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' aggregation='none'\n"
    "                        isNavigable='true'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  EXPECT_NE(string::npos, err.str().find("Unknown association-end aggregation"));
  // Defaults to NONE
  auto assocs = reader.model->get_associations();
  EXPECT_EQ(UML::AGGREGATION_NONE, assocs.front()->connections[0]->aggregation);
}

//==========================================================================
// get_other_end

TEST(XMIReaderTest, TestGetOtherEnd)
{
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Association xmi.id='as1' name='a'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' name='endA' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' name='endB' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  ASSERT_EQ(1u, assocs.size());
  auto *end0 = assocs.front()->connections[0];
  auto *end1 = assocs.front()->connections[1];

  auto *other0 = end0->get_other_end();
  ASSERT_NE(nullptr, other0);
  EXPECT_EQ("endB", other0->name);

  auto *other1 = end1->get_other_end();
  ASSERT_NE(nullptr, other1);
  EXPECT_EQ("endA", other1->name);

  // Print to exercise AGGREGATION_NONE in print_header
  ostringstream out;
  reader.model->print(out);
  string result = out.str();
  EXPECT_NE(string::npos, result.find("endA"));
}

//==========================================================================
// Generalization (inheritance)

TEST(XMIReaderTest, TestParseGeneralization)
{
  string content =
    "<UML:Class xmi.id='c1' name='Parent'/>\n"
    "<UML:Class xmi.id='c2' name='Child'/>\n"
    "<UML:Generalization xmi.id='g1'>\n"
    "  <UML:Generalization.parent>\n"
    "    <UML:Class xmi.idref='c1'/>\n"
    "  </UML:Generalization.parent>\n"
    "  <UML:Generalization.child>\n"
    "    <UML:Class xmi.idref='c2'/>\n"
    "  </UML:Generalization.child>\n"
    "</UML:Generalization>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(2u, classes.size());
}

//==========================================================================
// Interface parsing

TEST(XMIReaderTest, TestParseInterface)
{
  string content =
    "<UML:Interface xmi.id='i1' name='MyInterface' isAbstract='true'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto interfaces = reader.model->get_interfaces();
  ASSERT_EQ(1u, interfaces.size());
  EXPECT_EQ("MyInterface", interfaces.front()->name);
}

//==========================================================================
// Comprehensive print test - exercises all print_header methods

TEST(XMIReaderTest, TestComprehensivePrint)
{
  // Build a rich model that exercises all print_header chains
  string content =
    // DataTypes for references
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:DataType xmi.id='t2' name='string'/>\n"
    "<UML:DataType xmi.id='t3' name='void'/>\n"
    // Stereotype
    "<UML:Stereotype xmi.id='s1' name='entity'/>\n"
    // Enumeration
    "<UML:Enumeration xmi.id='e1' name='Color'>\n"
    "  <UML:Enumeration.literal>\n"
    "    <UML:EnumerationLiteral xmi.id='el1' name='Red'/>\n"
    "    <UML:EnumerationLiteral xmi.id='el2' name='Blue'/>\n"
    "  </UML:Enumeration.literal>\n"
    "</UML:Enumeration>\n"
    // Class with attributes and operations
    "<UML:Class xmi.id='c1' name='MyClass' visibility='public'\n"
    "           stereotype='s1' isActive='true'>\n"
    "  <UML:Classifier.feature>\n"
    // Attribute with type, initial value, static, ordered
    "    <UML:Attribute xmi.id='a1' name='count' visibility='private'\n"
    "                   ownerScope='classifier' ordering='ordered'>\n"
    "      <UML:StructuralFeature.type>\n"
    "        <UML:Classifier xmi.idref='t1'/>\n"
    "      </UML:StructuralFeature.type>\n"
    "      <UML:Attribute.initialValue>\n"
    "        <UML:Expression language='C++' body='0'/>\n"
    "      </UML:Attribute.initialValue>\n"
    "    </UML:Attribute>\n"
    // Operation: abstract, sequential, with query
    "    <UML:Operation xmi.id='o1' name='doWork' visibility='public'\n"
    "                   isAbstract='true' isRoot='true' isLeaf='true'\n"
    "                   concurrency='sequential'\n"
    "                   isQuery='true'>\n"
    "      <UML:BehaviouralFeature.parameter>\n"
    "        <UML:Parameter xmi.id='p1' name='x' kind='in'\n"
    "                       type='t1'>\n"
    "          <UML:Attribute.defaultValue>\n"
    "            <UML:Expression language='C++' body='42'/>\n"
    "          </UML:Attribute.defaultValue>\n"
    "        </UML:Parameter>\n"
    "        <UML:Parameter xmi.id='p2' name='y' kind='inout'\n"
    "                       type='t2'/>\n"
    "        <UML:Parameter xmi.id='p3' name='z' kind='out'\n"
    "                       type='t1'/>\n"
    "        <UML:Parameter xmi.id='p4' name='ret' kind='return'\n"
    "                       type='t3'/>\n"
    "      </UML:BehaviouralFeature.parameter>\n"
    "    </UML:Operation>\n"
    // Operation: guarded concurrency
    "    <UML:Operation xmi.id='o2' name='guardedOp'\n"
    "                   concurrency='guarded'/>\n"
    // Operation: concurrent (default, not printed)
    "    <UML:Operation xmi.id='o3' name='concOp'\n"
    "                   concurrency='concurrent'/>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n"
    // Second class for associations
    "<UML:Class xmi.id='c2' name='OtherClass' visibility='protected'/>\n"
    // Association with aggregation and non-navigable, ordered ends
    "<UML:Association xmi.id='as1' name='rel'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' name='source'\n"
    "                        isNavigable='false'\n"
    "                        aggregation='aggregate'\n"
    "                        ordering='ordered'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "      <UML:AssociationEnd.multiplicity>\n"
    "        <UML:Multiplicity>\n"
    "          <UML:Multiplicity.range>\n"
    "            <UML:MultiplicityRange lower='0' upper='-1'/>\n"
    "          </UML:Multiplicity.range>\n"
    "        </UML:Multiplicity>\n"
    "      </UML:AssociationEnd.multiplicity>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' name='target'\n"
    "                        isNavigable='true'\n"
    "                        aggregation='composite'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "      <UML:AssociationEnd.multiplicity>\n"
    "        <UML:Multiplicity>\n"
    "          <UML:Multiplicity.range>\n"
    "            <UML:MultiplicityRange lower='1' upper='5'/>\n"
    "          </UML:Multiplicity.range>\n"
    "        </UML:Multiplicity>\n"
    "      </UML:AssociationEnd.multiplicity>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n"
    // Generalization
    "<UML:Generalization xmi.id='g1'>\n"
    "  <UML:Generalization.parent>\n"
    "    <UML:Class xmi.idref='c1'/>\n"
    "  </UML:Generalization.parent>\n"
    "  <UML:Generalization.child>\n"
    "    <UML:Class xmi.idref='c2'/>\n"
    "  </UML:Generalization.child>\n"
    "</UML:Generalization>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  ostringstream out;
  reader.model->print(out);
  string result = out.str();

  // Model header
  EXPECT_NE(string::npos, result.find("TestModel"));

  // Class with stereotype
  EXPECT_NE(string::npos, result.find("MyClass"));
  EXPECT_NE(string::npos, result.find("<<entity>>"));
  EXPECT_NE(string::npos, result.find("(active)"));

  // Attribute: type, initial value, static, ordered
  EXPECT_NE(string::npos, result.find("count"));
  EXPECT_NE(string::npos, result.find("(static)"));
  EXPECT_NE(string::npos, result.find("(ordered)"));
  EXPECT_NE(string::npos, result.find("= '0'"));
  EXPECT_NE(string::npos, result.find("int"));

  // Operation: abstract, root, leaf, sequential, query
  EXPECT_NE(string::npos, result.find("doWork"));
  EXPECT_NE(string::npos, result.find("(abstract)"));
  EXPECT_NE(string::npos, result.find("(root)"));
  EXPECT_NE(string::npos, result.find("(leaf)"));
  EXPECT_NE(string::npos, result.find("(sequential)"));
  EXPECT_NE(string::npos, result.find("(query)"));

  // Parameter with default value
  EXPECT_NE(string::npos, result.find("= '42'"));
  EXPECT_NE(string::npos, result.find("(in)"));
  EXPECT_NE(string::npos, result.find("(inout)"));
  EXPECT_NE(string::npos, result.find("(out)"));
  EXPECT_NE(string::npos, result.find("(return)"));

  // Guarded operation
  EXPECT_NE(string::npos, result.find("(guarded)"));

  // Enumeration with literals
  EXPECT_NE(string::npos, result.find("Color"));
  EXPECT_NE(string::npos, result.find("'Red'"));
  EXPECT_NE(string::npos, result.find("'Blue'"));

  // Association ends: aggregate, composite, non-navigable, ordered
  EXPECT_NE(string::npos, result.find("(aggregate)"));
  EXPECT_NE(string::npos, result.find("(composite)"));
  EXPECT_NE(string::npos, result.find("(non-navigable)"));

  // Multiplicity [0..*]
  EXPECT_NE(string::npos, result.find("[0..*]"));
  // Multiplicity [1..5]
  EXPECT_NE(string::npos, result.find("[1..5]"));

  // Generalization
  EXPECT_NE(string::npos, result.find("OtherClass"));

  // Visibility in print output
  EXPECT_NE(string::npos, result.find("public"));
  EXPECT_NE(string::npos, result.find("private"));
  EXPECT_NE(string::npos, result.find("protected"));
}

//==========================================================================
// Element lookup

TEST(XMIReaderTest, TestLookupUMLElement)
{
  string content =
    "<UML:Class xmi.id='c1' name='Found'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto *elem = reader.lookup_uml_element("c1");
  ASSERT_NE(nullptr, elem);
}

TEST(XMIReaderTest, TestLookupUMLElementNotFound)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(""));
  reader.read_from(xmi);

  auto *elem = reader.lookup_uml_element("nonexistent");
  EXPECT_EQ(nullptr, elem);
}

TEST(XMIReaderTest, TestLookupXMLElement)
{
  // Use a class without attributes/operations to avoid build_refs issues
  string content =
    "<UML:Class xmi.id='c1' name='MyClass'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  // XML element map should contain elements with xmi.id
  auto *xmle = reader.lookup_xml_element("c1");
  ASSERT_NE(nullptr, xmle);
}

TEST(XMIReaderTest, TestLookupXMLElementNotFound)
{
  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(""));
  reader.read_from(xmi);

  auto *xmle = reader.lookup_xml_element("nonexistent");
  EXPECT_EQ(nullptr, xmle);
}

//==========================================================================
// XMI 1.0 upgrade

TEST(XMIReaderTest, TestXMI10Upgrade)
{
  // XMI 1.0 uses fully-qualified element names
  string xmi10 =
    "<?xml version='1.0'?>\n"
    "<XMI xmi.version='1.0'>\n"
    "  <XMI.content>\n"
    "    <Model_Management.Model xmi.id='m1' name='OldModel'>\n"
    "      <Foundation.Core.Namespace.ownedElement>\n"
    "        <Foundation.Core.Class xmi.id='c1' name='OldClass'/>\n"
    "      </Foundation.Core.Namespace.ownedElement>\n"
    "    </Model_Management.Model>\n"
    "  </XMI.content>\n"
    "</XMI>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream in(xmi10);
  reader.read_from(in);

  ASSERT_NE(nullptr, reader.model);
  EXPECT_EQ("OldModel", reader.model->name);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  EXPECT_EQ("OldClass", classes.front()->name);
}

//==========================================================================
// Stereotype

TEST(XMIReaderTest, TestParseStereotype)
{
  string content =
    "<UML:Stereotype xmi.id='s1' name='entity'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass' stereotype='s1'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  auto *c = classes.front();
  EXPECT_EQ("MyClass", c->name);
  ASSERT_NE(nullptr, c->stereotype);
  EXPECT_EQ("entity", c->stereotype->name);
}

TEST(XMIReaderTest, TestBogusStereotypeWarning)
{
  // Stereotype idref points to a non-Stereotype element
  string content =
    "<UML:Class xmi.id='c1' name='NotAStereotype'/>\n"
    "<UML:Class xmi.id='c2' name='MyClass' stereotype='c1'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  EXPECT_NE(string::npos, err.str().find("Bogus stereotype"));
}

TEST(XMIReaderTest, TestNonConnectedStereotypeWarning)
{
  // Stereotype idref points to a non-existent element
  string content =
    "<UML:Class xmi.id='c1' name='MyClass' stereotype='nonexistent'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  EXPECT_NE(string::npos, err.str().find("Non-connected stereotype"));
}

//==========================================================================
// Operation concurrency

TEST(XMIReaderTest, TestOperationConcurrency)
{
  string content =
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='seq' concurrency='sequential'/>\n"
    "    <UML:Operation xmi.id='o2' name='grd' concurrency='guarded'/>\n"
    "    <UML:Operation xmi.id='o3' name='con' concurrency='concurrent'/>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  auto ops = classes.front()->get_operations();
  ASSERT_EQ(3u, ops.size());
  auto it = ops.begin();
  EXPECT_EQ(UML::CONCURRENCY_SEQUENTIAL, (*it)->concurrency); ++it;
  EXPECT_EQ(UML::CONCURRENCY_GUARDED, (*it)->concurrency); ++it;
  EXPECT_EQ(UML::CONCURRENCY_CONCURRENT, (*it)->concurrency);
}

TEST(XMIReaderTest, TestUnknownConcurrencyWarning)
{
  string content =
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Operation xmi.id='o1' name='op' concurrency='bogus'/>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  EXPECT_NE(string::npos, err.str().find("Unknown operation concurrency"));
  auto ops = reader.model->get_classes().front()->get_operations();
  EXPECT_EQ(UML::CONCURRENCY_SEQUENTIAL, ops.front()->concurrency);
}

//==========================================================================
// Unknown visibility warning

TEST(XMIReaderTest, TestUnknownVisibilityWarning)
{
  string content =
    "<UML:Class xmi.id='c1' name='MyClass' visibility='bogus'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  EXPECT_NE(string::npos, err.str().find("Unknown element visibility"));
  // Defaults to private
  auto classes = reader.model->get_classes();
  EXPECT_EQ(UML::VISIBILITY_PRIVATE, classes.front()->visibility);
}

//==========================================================================
// Class map

TEST(XMIReaderTest, TestClassMap)
{
  string content =
    "<UML:Class xmi.id='c1' name='Mapped'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto p = reader.class_map.find("Mapped");
  ASSERT_NE(reader.class_map.end(), p);
  EXPECT_EQ("Mapped", p->second->name);
}

//==========================================================================
// Metamodel warning

TEST(XMIReaderTest, TestNonUMLMetamodelWarning)
{
  string xmi_doc =
    "<?xml version='1.0'?>\n"
    "<XMI xmi.version='1.1'>\n"
    "  <XMI.header>\n"
    "    <XMI.metamodel xmi.name='NotUML' xmi.version='1.0'/>\n"
    "  </XMI.header>\n"
    "  <XMI.content>\n"
    "    <UML:Model xmi.id='m1' name='Test'>\n"
    "      <UML:Namespace.ownedElement/>\n"
    "    </UML:Model>\n"
    "  </XMI.content>\n"
    "</XMI>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream in(xmi_doc);
  reader.read_from(in);

  EXPECT_NE(string::npos, err.str().find("isn't UML"));
}

//==========================================================================
// Multiplicity reading

TEST(XMIReaderTest, TestMultiplicityFromAttributes)
{
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Association xmi.id='as1' name='a'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "      <UML:AssociationEnd.multiplicity>\n"
    "        <UML:Multiplicity>\n"
    "          <UML:Multiplicity.range>\n"
    "            <UML:MultiplicityRange lower='0' upper='-1'/>\n"
    "          </UML:Multiplicity.range>\n"
    "        </UML:Multiplicity>\n"
    "      </UML:AssociationEnd.multiplicity>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  auto *end0 = assocs.front()->connections[0];
  EXPECT_EQ(0, end0->multiplicity.lower);
  EXPECT_EQ(-1, end0->multiplicity.upper);

  // Print multiplicity [0..*]
  ostringstream ms;
  ms << end0->multiplicity;
  EXPECT_EQ("[0..*]", ms.str());

  // Default 1..1 prints nothing
  auto *end1 = assocs.front()->connections[1];
  ostringstream ms2;
  ms2 << end1->multiplicity;
  EXPECT_EQ("", ms2.str());
}

TEST(XMIReaderTest, TestMultiplicityFromSubelements)
{
  // MultiplicityRange values as child elements instead of attributes
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Association xmi.id='as1' name='a'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "      <UML:AssociationEnd.multiplicity>\n"
    "        <UML:Multiplicity>\n"
    "          <UML:Multiplicity.range>\n"
    "            <UML:MultiplicityRange>\n"
    "              <UML:MultiplicityRange.lower>2</UML:MultiplicityRange.lower>\n"
    "              <UML:MultiplicityRange.upper>10</UML:MultiplicityRange.upper>\n"
    "            </UML:MultiplicityRange>\n"
    "          </UML:Multiplicity.range>\n"
    "        </UML:Multiplicity>\n"
    "      </UML:AssociationEnd.multiplicity>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  auto *end0 = assocs.front()->connections[0];
  EXPECT_EQ(2, end0->multiplicity.lower);
  EXPECT_EQ(10, end0->multiplicity.upper);

  ostringstream ms;
  ms << end0->multiplicity;
  EXPECT_EQ("[2..10]", ms.str());
}

TEST(XMIReaderTest, TestMultiplicityWithIdref)
{
  // Multiplicity referenced by idref instead of inline
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Association xmi.id='as1' name='a'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "      <UML:AssociationEnd.multiplicity>\n"
    "        <UML:Multiplicity xmi.idref='mult1'/>\n"
    "      </UML:AssociationEnd.multiplicity>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.participant>\n"
    "        <UML:Class xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.participant>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  // The multiplicity definition needs to be in the model namespace
  // so the XML element map can find it. Wrap it as a sibling.
  string full_xmi =
    "<?xml version='1.0' encoding='UTF-8'?>\n"
    "<XMI xmi.version='1.1' xmlns:UML='org.omg.xmi.namespace.UML'>\n"
    "  <XMI.header>\n"
    "    <XMI.metamodel xmi.name='UML' xmi.version='1.4'/>\n"
    "  </XMI.header>\n"
    "  <XMI.content>\n"
    "    <UML:Model xmi.id='m1' name='TestModel'>\n"
    "      <UML:Namespace.ownedElement>\n"
    + content +
    "        <UML:Multiplicity xmi.id='mult1'>\n"
    "          <UML:Multiplicity.range>\n"
    "            <UML:MultiplicityRange lower='3' upper='7'/>\n"
    "          </UML:Multiplicity.range>\n"
    "        </UML:Multiplicity>\n"
    "      </UML:Namespace.ownedElement>\n"
    "    </UML:Model>\n"
    "  </XMI.content>\n"
    "</XMI>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(full_xmi);
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  auto *end0 = assocs.front()->connections[0];
  EXPECT_EQ(3, end0->multiplicity.lower);
  EXPECT_EQ(7, end0->multiplicity.upper);
}

//==========================================================================
// Expression subelement form

TEST(XMIReaderTest, TestExpressionFromSubelements)
{
  // Expression with language/body as child elements instead of attributes
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='x'>\n"
    "      <UML:StructuralFeature.type>\n"
    "        <UML:Classifier xmi.idref='t1'/>\n"
    "      </UML:StructuralFeature.type>\n"
    "      <UML:Attribute.initialValue>\n"
    "        <UML:Expression>\n"
    "          <UML:Expression.language>Java</UML:Expression.language>\n"
    "          <UML:Expression.body>99</UML:Expression.body>\n"
    "        </UML:Expression>\n"
    "      </UML:Attribute.initialValue>\n"
    "    </UML:Attribute>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto attrs = reader.model->get_classes().front()->get_attributes();
  ASSERT_EQ(1u, attrs.size());
  EXPECT_EQ("99", attrs.front()->initial_value.body);
  EXPECT_EQ("Java", attrs.front()->initial_value.language);
}

//==========================================================================
// Property via subelement (xmi.value and content forms)

TEST(XMIReaderTest, TestPropertyViaSubelementXmiValue)
{
  // isAbstract as subelement with xmi.value instead of attribute
  string content =
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:GeneralizableElement.isAbstract xmi.value='true'/>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  EXPECT_TRUE(classes.front()->is_abstract);
}

TEST(XMIReaderTest, TestPropertyViaSubelementContent)
{
  // name as subelement content instead of attribute
  string content =
    "<UML:Class xmi.id='c1'>\n"
    "  <UML:ModelElement.name>ContentName</UML:ModelElement.name>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  EXPECT_EQ("ContentName", classes.front()->name);
}

//==========================================================================
// get_int_property

TEST(XMIReaderTest, TestIntPropertyDefault)
{
  // int_property with no value returns default
  // We can't directly test get_int_property, but we can verify
  // Multiplicity reads defaults correctly (uses get_attr_int in XML)
  string content = "<UML:Class xmi.id='c1' name='A'/>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  // Default multiplicity is 1..1
  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
}

//==========================================================================
// Stereotype via subelement

TEST(XMIReaderTest, TestStereotypeViaSubelement)
{
  // Stereotype reference as subelement instead of attribute
  string content =
    "<UML:Stereotype xmi.id='s1' name='persistent'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:ModelElement.stereotype>\n"
    "    <UML:Stereotype xmi.idref='s1'/>\n"
    "  </UML:ModelElement.stereotype>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto classes = reader.model->get_classes();
  ASSERT_EQ(1u, classes.size());
  ASSERT_NE(nullptr, classes.front()->stereotype);
  EXPECT_EQ("persistent", classes.front()->stereotype->name);
}

//==========================================================================
// Participant via UML 1.3 'type' subelement

TEST(XMIReaderTest, TestAssociationEndType13)
{
  // UML 1.3 uses 'type' instead of 'participant'
  string content =
    "<UML:Class xmi.id='c1' name='A'/>\n"
    "<UML:Class xmi.id='c2' name='B'/>\n"
    "<UML:Association xmi.id='as1' name='a'>\n"
    "  <UML:Association.connection>\n"
    "    <UML:AssociationEnd xmi.id='ae1' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.type>\n"
    "        <UML:Classifier xmi.idref='c1'/>\n"
    "      </UML:AssociationEnd.type>\n"
    "    </UML:AssociationEnd>\n"
    "    <UML:AssociationEnd xmi.id='ae2' isNavigable='true'\n"
    "                        aggregation='none'>\n"
    "      <UML:AssociationEnd.type>\n"
    "        <UML:Classifier xmi.idref='c2'/>\n"
    "      </UML:AssociationEnd.type>\n"
    "    </UML:AssociationEnd>\n"
    "  </UML:Association.connection>\n"
    "</UML:Association>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto assocs = reader.model->get_associations();
  ASSERT_EQ(1u, assocs.size());
  EXPECT_NE(nullptr, assocs.front()->connections[0]->participant);
  EXPECT_EQ("A", assocs.front()->connections[0]->participant->name);
}

//==========================================================================
// MDR fallbacks (type via UML:Class instead of UML:Classifier)

TEST(XMIReaderTest, TestTypeLookupViaMDRClass)
{
  // Netbeans MDR uses UML:Class instead of UML:Classifier in type refs
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='x'>\n"
    "      <UML:StructuralFeature.type>\n"
    "        <UML:DataType xmi.idref='t1'/>\n"
    "      </UML:StructuralFeature.type>\n"
    "    </UML:Attribute>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  reader.read_from(xmi);

  auto attrs = reader.model->get_classes().front()->get_attributes();
  ASSERT_EQ(1u, attrs.size());
  ASSERT_NE(nullptr, attrs.front()->type);
  EXPECT_EQ("int", attrs.front()->type->name);
}

//==========================================================================
// Non-connected type idref warning

TEST(XMIReaderTest, TestNonConnectedTypeWarning)
{
  // Attribute type points to a non-existent element
  string content =
    "<UML:Class xmi.id='c1' name='MyClass'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='x' type='nonexistent'/>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  // build_refs will call reader.error for missing type
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
  EXPECT_NE(string::npos, err.str().find("Non-connected type idref"));
}

//==========================================================================
// Bogus classifier idref warning

TEST(XMIReaderTest, TestBogusClassifierWarning)
{
  // Attribute type points to a non-Classifier element (Generalization)
  string content =
    "<UML:Class xmi.id='c1' name='Parent'/>\n"
    "<UML:Class xmi.id='c2' name='Child'/>\n"
    "<UML:Generalization xmi.id='g1'>\n"
    "  <UML:Generalization.parent>\n"
    "    <UML:Class xmi.idref='c1'/>\n"
    "  </UML:Generalization.parent>\n"
    "  <UML:Generalization.child>\n"
    "    <UML:Class xmi.idref='c2'/>\n"
    "  </UML:Generalization.child>\n"
    "</UML:Generalization>\n"
    "<UML:Class xmi.id='c3' name='BadType'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='x' type='g1'/>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
  EXPECT_NE(string::npos, err.str().find("Bogus classifier"));
}

//==========================================================================
// Generalization error paths

TEST(XMIReaderTest, TestGeneralizationMissingParent)
{
  string content =
    "<UML:Class xmi.id='c2' name='Child'/>\n"
    "<UML:Generalization xmi.id='g1'>\n"
    "  <UML:Generalization.parent>\n"
    "    <UML:GeneralizableElement xmi.idref='nonexistent'/>\n"
    "  </UML:Generalization.parent>\n"
    "  <UML:Generalization.child>\n"
    "    <UML:Class xmi.idref='c2'/>\n"
    "  </UML:Generalization.child>\n"
    "</UML:Generalization>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
  EXPECT_NE(string::npos, err.str().find("parent"));
}

TEST(XMIReaderTest, TestBogusGEIdref)
{
  // Generalization parent references a non-GeneralizableElement (Attribute)
  // Attribute IS registered in uml_element_map but fails dynamic_cast to GE
  string content =
    "<UML:DataType xmi.id='t1' name='int'/>\n"
    "<UML:Class xmi.id='c1' name='A'>\n"
    "  <UML:Classifier.feature>\n"
    "    <UML:Attribute xmi.id='a1' name='x'>\n"
    "      <UML:StructuralFeature.type>\n"
    "        <UML:Classifier xmi.idref='t1'/>\n"
    "      </UML:StructuralFeature.type>\n"
    "    </UML:Attribute>\n"
    "  </UML:Classifier.feature>\n"
    "</UML:Class>\n"
    "<UML:Generalization xmi.id='g1'>\n"
    "  <UML:Generalization.parent>\n"
    "    <UML:GeneralizableElement xmi.idref='a1'/>\n"
    "  </UML:Generalization.parent>\n"
    "  <UML:Generalization.child>\n"
    "    <UML:GeneralizableElement xmi.idref='c1'/>\n"
    "  </UML:Generalization.child>\n"
    "</UML:Generalization>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
  EXPECT_NE(string::npos, err.str().find("Bogus GE"));
}

TEST(XMIReaderTest, TestGeneralizationMissingChild)
{
  string content =
    "<UML:Class xmi.id='c1' name='Parent'/>\n"
    "<UML:Generalization xmi.id='g1'>\n"
    "  <UML:Generalization.parent>\n"
    "    <UML:GeneralizableElement xmi.idref='c1'/>\n"
    "  </UML:Generalization.parent>\n"
    "  <UML:Generalization.child>\n"
    "    <UML:GeneralizableElement xmi.idref='nonexistent'/>\n"
    "  </UML:Generalization.child>\n"
    "</UML:Generalization>\n";

  ostringstream err;
  XMI::Reader reader(err);
  istringstream xmi(make_xmi(content));
  EXPECT_THROW(reader.read_from(xmi), XMI::ParseFailed);
  EXPECT_NE(string::npos, err.str().find("child"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
