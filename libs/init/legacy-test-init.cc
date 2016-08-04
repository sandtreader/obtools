//==========================================================================
// ObTools::Init: test-init.cc
//
// Test harness for initialisation library functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-init.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test types
class Super
{
public:
  int a;
  Super(int _a): a(_a) {}
};

class Sub: public Super
{
public:
  Sub(int _a): Super(_a+1) {}
};

Init::Registry<Super, int> sr;
static Init::AutoRegister<Super, Sub, int> ar(sr, "sub");

//--------------------------------------------------------------------------
// Main
int main()
{
  Init::Sequence::run();

  Super *s = sr.create("sub", 98);

  cout << "Sub: " << s->a << endl;
  delete s;
  return 0;
}




