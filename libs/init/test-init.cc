//==========================================================================
// ObTools::Init: test-init.cc
//
// Test harness for initialisation library functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-init.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test types
class Fred
{
public:
  int a;
  Fred(int _a): a(_a) {}

  // Create factory method
  static Fred *create(int a) { return new Fred(a); }
};

Init::Registry<Fred, int> fr;
static Init::AutoRegister<Fred, int> ar(fr, "fred");

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  Init::Sequence::run();

  Fred *f = fr.create("fred", 99);


  cout << "Fred: " << f->a << endl;
  return 0;  
}




