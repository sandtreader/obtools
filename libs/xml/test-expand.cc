//==========================================================================
// ObTools::XML: test-expand.cc
//
// Test harness for ObTools XML expansion functionality
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

  // Create an expansion map
  map<string, string> expansions;
  expansions["foo"] = "Maybe!";
  expansions["true"] = "TRUE";
  expansions["wombat0"] = "a small furry animal";
  expansions["latin0"]  = "Vombatus primus";

  expansions["wombat1"] = "another small furry animal";
  expansions["latin1"]  = "Vombatus secundus";

  expansions["wombat2"] = "SFA#3";
  expansions["latin2"]  = "Vombatus tertius";

  // Expand
  ObTools::XML::Expander expander(expansions);
  cout << "Expanded element:\n" << expander.expand(root);

  return 0;  
}




