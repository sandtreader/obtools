//==========================================================================
// ObTools::CLI: test-telnet.cc
//
// Test harness for Telnet command-line server
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
#ifdef __WIN32__
  winsock_initialise();
#endif

  // Create command registry
  CLI::Registry reg;
  reg.add("foo", new FooHandler(), "Launch foos", "foo <options>");

  // Create and run command line
  CLI::TelnetServer cli(reg, 7777, "Test#");
  cli.run();

  return 0;  
}




