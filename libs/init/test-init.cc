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
int main(int argc, char **argv)
{
  Init::Sequence::run();

  Super *s = sr.create("sub", 98);

  cout << "Sub: " << s->a << endl;
  return 0;  
}



