//==========================================================================
// ObTools::XMI: test-xmi.cc
//
// Test harness for ObTools XMI reader
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmi.h"

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::XMI::Reader reader;

  try
  {
    cin >> reader;
  }
  catch (ObTools::XMI::ParseFailed)
  {
    cerr << "XMI parse failed" << endl;
    return 2;
  }

  //Show versions
  cout << "XMI version: " << reader.xmi_version << endl;
  cout << "UML version: " << reader.uml_version << endl;

  //List model
  if (reader.model) reader.model->print(cout);

  return 0;  
}




