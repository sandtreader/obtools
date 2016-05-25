//==========================================================================
// ObTools::XML: test-config.cc
//
// Test harness for ObTools XML Configuration file support
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  list<string> filenames;
  filenames.push_back("not-there.xml");
  filenames.push_back("../tests/config.xml");
  filenames.push_back("../tests/simple.xml");
  ObTools::XML::Configuration config(filenames);

  if (!config.read("config")) return 2;

  cout << "/@version: "  << config.get_value("/@version") << endl;
  cout << "directory: "  << config.get_value("directory") << endl;
  cout << "output/mode: "  << config.get_value_int("output/mode", 444) << endl;
  cout << "output/atomic: " 
       << config.get_value_bool("output/atomic", false) << endl;

  //Files
  list<string> files = config.get_values("input/file");
  for(list<string>::iterator p = files.begin();
      p!=files.end();
      p++)
    cout << "input/file: " << *p << endl;

  //Map
  map<string, string> m = config.get_map("types/map");
  for(map<string, string>::iterator p = m.begin(); p!=m.end(); p++)
    cout << "Map '" << p->first << "'->'" << p->second << "'\n";

  return 0;  
}




