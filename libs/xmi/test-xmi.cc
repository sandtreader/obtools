//==========================================================================
// ObTools::XMI: test-xmi.cc
//
// Test harness for ObTools XMI reader
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "xmi.h"

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
  return 0;  
}




