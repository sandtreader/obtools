//==========================================================================
// ObTools::XML: test-xpath.cc
//
// Test harness for ObTools XML Parser XPath processing
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
  list<ObTools::XML::Element *> l;

  try
  {
    cin >> parser;
  }
  catch (ObTools::XML::ParseFailed)
  {
    cerr << "XML parse failed" << endl;
    return 2;
  }

  ObTools::XML::XPathProcessor xpath(parser.get_root());

  l = xpath.get_elements("/XX1:bing/XX1:bong/wombat");

  for(list<ObTools::XML::Element *>::const_iterator p=l.begin();
      p!=l.end();
      p++)
  {
    ObTools::XML::Element& wombat=**p;
    cout << "  [" << wombat.content << "]" << endl;
  }

  cout << "/: "             << xpath.get_value("/") << endl;
  cout << "@attr2: "        << xpath.get_value("@attr2") << endl;
  cout << "/@attr3: "       << xpath.get_value("/@attr3") << endl;
  cout << "/wombat: "       << xpath.get_value("/wombat") << endl;
  cout << "cellar/wombat: " << xpath.get_value("cellar/wombat") << endl;
  cout << "XX1:bing/XX1:bong/@jim: " 
       << xpath.get_value_int("XX1:bing/XX1:bong/@jim") << endl;
  cout << "XX1:bing/XX1:bong/random:element/@flag: " 
       << xpath.get_value_bool("XX1:bing/XX1:bong/random:element/@flag") 
       << endl;
  cout << "not: "           << xpath.get_value("not", "OK") << endl;
  cout << "not/@foo: "      << xpath.get_value("not/@foo", "OK") << endl;
  cout << "bar/@not: "      << xpath.get_value("bar/@not", "OK") << endl;

  return 0;  
}




