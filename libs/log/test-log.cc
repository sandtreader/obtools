//==========================================================================
// ObTools::Log: test-log.cc
//
// Test harness for log library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  //!!!  ObTools::Log::logger.connect(new StreamChannel(cout));

  ObTools::Log::Summary << "Hello, world\n";

  if (ObTools::Log::dump_ok)
    ObTools::Log::Dump << "This is more than you ever wanted to know\n";

  return 0;  
}




