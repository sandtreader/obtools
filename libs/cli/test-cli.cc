//==========================================================================
// ObTools::CLI: test-cli.cc
//
// Test harness for command-line library
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-cli.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
//Foo command handler
class FooHandler: public CLI::Handler
{
public:
  void handle(string args, istream& sin, ostream& sout)
  {
    sout << "FOO [" << args << "]!\n";
  }
};

//--------------------------------------------------------------------------
//Complex class with member handlers
class BigClass
{
public:
  string t;
  BigClass(const string& _t): t(_t) {}

  void fred_handler(string args, istream& sin, ostream& sout)
  { sout << "Fred " << t << endl; }

  void jim_handler(string args, istream& sin, ostream& sout)
  { sout << "Jim " << t << endl; }
};

//--------------------------------------------------------------------------
// Main

int main()
{
  BigClass c("hello");

  // Create command registry
  CLI::Registry reg;
  reg.add("foo", new FooHandler(), "Launch foos", "foo <options>");
  reg.add("jim", 
	  new CLI::MemberHandler<BigClass>(c, &BigClass::jim_handler),
	  "Ask jim");
  reg.add("fred", 
	  new CLI::MemberHandler<BigClass>(c, &BigClass::fred_handler),
	  "Ask fred");

  // Create and command line
  CLI::CommandLine cli(reg, cin, cout, "Test>");
  cli.run();

  return 0;  
}




