//==========================================================================
// ObTools::CPPT: test-cppt.cc
//
// Test harness for C++ template processor
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-cppt.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::CPPT::Tags tags = 
  { "<?", "?>", "<?=", "?>", "<?#", "?>" };

  ObTools::CPPT::Processor processor(cin, cout, tags, "cout");
  cout << "#include <iostream>\n";
  cout << "using namespace std;\n\n";
  cout << "int main()\n";
  cout << "{\n";

  processor.process();

  cout << "return 0;\n";
  cout << "}\n";

  return 0;  
}




