//==========================================================================
// ObTools::XML: test-update.cc
//
// Test harness for ObTools XML Configuration file update support
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  list<string> filenames;
  filenames.push_back("../tests/config.xml");
  ObTools::XML::Configuration config(filenames);
  if (!config.read("config")) return 2;

  // Add some stuff
  config.ensure_path("extra/stuff");
  config.set_value("extra/stuff/@arg", "Added attribute");
  config.add_element("extra/stuff", "more");
  config.set_value_bool("extra/stuff/more/@bool", true);
  config.set_value_int("extra/stuff/more/@int", 42);
  config.set_value_real("extra/stuff/more/@real", 3.1415926);
  config.set_value("extra/stuff/more", "Added content");

  // Print it out
  cout << config.get_root();

  // Don't update - it messes up CVS!
  //config.write();

  return 0;  
}




