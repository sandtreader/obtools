//==========================================================================
// ObTools::XML: test-xml.cc
//
// Test harness for ObTools XML Parser
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-xml.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::XML::Parser parser;

  //Create some test namespaces
  parser.fix_namespace("namespace1", "ns1");
  parser.fix_namespace("namespace2", "ns2");

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

  cout << "This is a " << root.name << " document " << endl;
  cout << "root.attr1 is " << root.get_attr("attr1", "EMPTY") << endl;

  //Look for first wombat
  ObTools::XML::Element& wombat = root.get_child("wombat");
  if (wombat.valid())
    cout << "Houston, we have a wombat: [" << wombat.content << "]" << endl;
  else
    cout << "No wombats today!" << endl;

  //Look for lots of wombats
  list<ObTools::XML::Element *> wombats = root.get_children("wombat");
  if (wombats.size())
  {
    cout << "Wombats on the menu today:" << endl;
    for(list<ObTools::XML::Element *>::const_iterator p=wombats.begin();
	p!=wombats.end();
	p++)
    {
      ObTools::XML::Element& wombat=**p;
      cout << "  [" << wombat.content << "]" << endl;
    }
  }

  //Look for deeply-buried wombats
  wombats = root.get_descendants("wombat");
  if (wombats.size())
  {
    cout << "Wombats in the cellars today:" << endl;
    for(list<ObTools::XML::Element *>::const_iterator p=wombats.begin();
	p!=wombats.end();
	p++)
    {
      ObTools::XML::Element& wombat=**p;
      cout << "  [" << wombat.content << "]" << endl;
    }
  }

  cout << endl << "Here it is back again (with namespaces fixed):" << endl;
  cout << root;

  //Now translate it
  map<string,string> trans_map;
  trans_map["wombat"]="animal.small.furry";  // Translate
  trans_map["ns1:bing"]="";                  // Delete
  
  root.translate(trans_map);

  cout << endl;
  cout << "Here it is translated (wombats renamed, ns1:bing removed):" << endl;
  cout << root;

  return 0;  
}




