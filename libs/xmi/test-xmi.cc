//==========================================================================
// ObTools::XMI: test-xmi.cc
//
// Test harness for ObTools XMI reader
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmi.h"
using namespace std;

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

  if (!reader.model) return 4;

  //Show versions
  cout << "XMI version: " << reader.xmi_version << endl;
  cout << "UML version: " << reader.model->uml_version << endl;

  //List model
  reader.model->print(cout);

  return 0;  
}




