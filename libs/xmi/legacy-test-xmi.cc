//==========================================================================
// ObTools::XMI: test-xmi.cc
//
// Test harness for ObTools XMI reader
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  catch (const ObTools::XMI::ParseFailed&)
  {
    cerr << "XMI parse failed" << endl;
    return 2;
  }

  if (!reader.model) return 4;

  // Show versions
  cout << "XMI version: " << reader.xmi_version << endl;
  cout << "UML version: " << reader.model->uml_version << endl;

  // List model
  reader.model->print(cout);

  return 0;
}




