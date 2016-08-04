//==========================================================================
// ObTools::XML: test-expand.cc
//
// Test harness for ObTools XML expansion functionality
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::XML::Parser parser(ObTools::XML::PARSER_PRESERVE_WHITESPACE);

  try
  {
    cin >> parser;
  }
  catch (ObTools::XML::ParseFailed)
  {
    cerr << "XML parse failed" << endl;
    return 2;
  }

  ObTools::XML::Element& root=parser.get_root();

  cout << "Original element:\n" << root;

  // Create a values document
  ObTools::XML::Element values("values");
  values.add("foo", "Maybe!");
  values.add("test", "true", "Yes");

  values.add("wombat", "a small furry animal")
    .set_attr("latin", "Vombatus primus");

  values.add("wombat", "another small furry animal")
    .set_attr("latin", "Vombatus secundus");

  values.add("wombat", "SFA#3")
    .set_attr("latin", "Vombatus tertius");

  // Expand
  ObTools::XML::Expander expander(root);
  cout << "Expanded element:\n" << expander.expand(values);

  return 0;
}




