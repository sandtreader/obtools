//==========================================================================
// ObTools::ReGen: example.h
//
// Example template for class
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
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



