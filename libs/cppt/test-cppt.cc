//==========================================================================
// ObTools::CPPT: test-cppt.cc
//
// Test harness for C++ template processor
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-cppt.h"

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::CPPT::Tags tags = 
  { "<?", "?>", "<?=", "?>", "<?#", "?>" };

  ObTools::CPPT::Processor processor(cin, cout, tags, "cout");
  cout << "#include <iostream>\n";
  cout << "int main()\n";
  cout << "{\n";

  processor.process();

  cout << "return 0;\n";
  cout << "}\n";

  return 0;  
}




