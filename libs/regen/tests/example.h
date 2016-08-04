//==========================================================================
// ObTools::ReGen: example.h
//
// Example template for class
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

// These lines are perverse and should be ignored
//~
//~foo

//~[ 	  class foo
//------------------------------------------------------------------
class foo: bar
{

private:
  int a;
  string b;

  //~^ ----- Manually added private methods ------


  //~v -------------------------------------------

public:
  int uml_method();
  bar();
}
//~]


//~[ Example::foo (1)
//------------------------------------------------------------------------
void Example::foo(int x, int y)
{
//~^ ---- Manual code ----




//~v ---------------------
}
//~]



