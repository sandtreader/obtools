//==========================================================================
// ObTools::CLI: test-telnet.cc
//
// Test harness for Telnet command-line server
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-cli-telnet.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
//Foo command handler
class FooHandler: public CLI::Handler
{
public:
  void handle(string, istream& sin, ostream& sout)
  {
    sout << "Type something:\n";
    string s;
    getline(sin, s);
    sout << "You typed: " << s << endl;
  }
};

//--------------------------------------------------------------------------
// Main

int main()
{
  // Create command registry
  CLI::Registry reg;
  reg.add("foo", new FooHandler(), "Launch foos", "foo <options>");

  // Create and run command line
  CLI::TelnetServer cli(reg, 7777, "Test#");
  cli.run();

  return 0;  
}




