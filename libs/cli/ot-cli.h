//==========================================================================
// ObTools::CLI: ot-cli.h
//
// Public definitions for ObTools::CLI
// Command-line handling functionality
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_CLI_H
#define __OBTOOLS_CLI_H

#include <string>
#include <list>
#include <map>
#include <iostream>

namespace ObTools { namespace CLI { 

//Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
// Command handler - virtual interface
// Inherit this directly for single-command objects
//
// e.g.  class FooHandler: public Handler
//       {  .. implementation of handle() ... }
//
//       cli.register("foo", new FooHandler());

class Handler
{
public:
  //------------------------------------------------------------------------
  //Command handler function
  virtual void handle(string args, istream& sin, ostream& sout) = 0;
};

//==========================================================================
// Command registration template for object members
// Use this for more complex objects with multiple commands
//
// e.g. cli.register("foo", new MemberHandler<MyClass>(*this, &foo_handler));
//      cli.register("bar", new MemberHandler<MyClass>(*this, &bar_handler));

template<class T> class MemberHandler: public Handler
{
public:
  T& object;
  typedef void (T::*HandlerFunction)(string, istream&, ostream&);
  HandlerFunction func;

  //------------------------------------------------------------------------
  //Constructor
  MemberHandler(T& _object, HandlerFunction _func):
    object(_object), func(_func) {}

  //------------------------------------------------------------------------
  //Handler function
  void handle(string args, istream& sin, ostream& sout)
  { (object.*func)(args, sin, sout); }
};

//==========================================================================
// Command handler
class Command
{
private:
  Handler *handler;

public:
  string word;     // Command word we implement
  string help;     // Short help text
  string usage;    // Longer usage - only provide if it takes arguments

  //------------------------------------------------------------------------
  //Constructor
  Command(const string& _word, Handler *_handler, 
	  const string& _help="", const string& _usage=""):
    word(_word), handler(_handler), help(_help), usage(_usage) {}

  //------------------------------------------------------------------------
  //Handle a command
  //Note - overridden in CommandGroup
  virtual void handle(string args, istream& sin, ostream& sout);

  //------------------------------------------------------------------------
  //Show command usage
  void show_usage(ostream& sout);

  //------------------------------------------------------------------------
  //Destructor
  virtual ~Command() { if (handler) delete handler; }
};

//==========================================================================
// Command Group - group of commands with common prefix
class CommandGroup: public Command   // So we can recurse
{
public:
  map<string, Command *> commands;

  //------------------------------------------------------------------------
  //Constructor
  CommandGroup(const string& _word, const string& _help=""): 
    Command(_word, 0, _help) {}

  //------------------------------------------------------------------------
  //Add a command
  void add(Command *command);

  //------------------------------------------------------------------------
  //Handle a command
  virtual void handle(string args, istream& sin, ostream& sout);

  //------------------------------------------------------------------------
  //List help for the group
  void show_help(ostream& sout);

  //------------------------------------------------------------------------
  //Destructor
  virtual ~CommandGroup();
};

//==========================================================================
// Command Registry
class Registry: public CommandGroup
{
public:
  Registry(): CommandGroup("") {}

  //------------------------------------------------------------------------
  //Add a command-group
  void add(const string& prefix, const string& help="")
  { CommandGroup::add(new CommandGroup(prefix, help)); }

  //------------------------------------------------------------------------
  //Add a command
  void add(const string& word, Handler *handler, 
	   const string& help="", const string& usage="")
  { CommandGroup::add(new Command(word, handler, help, usage)); }

  //------------------------------------------------------------------------
  //Handle a command
  virtual void handle(string args, istream& sin, ostream& sout);
};

//==========================================================================
// Command line 
// Standard single command-line for IO streams (most usually, cin and cout)
class CommandLine
{
  Registry& registry;
  istream& sin;
  ostream& sout;

public:
  string prompt;

  //------------------------------------------------------------------------
  //Constructor
  CommandLine(Registry& _registry, istream& _sin, ostream& _sout, 
	      const string& _prompt=">"):
    registry(_registry), sin(_sin), sout(_sout), prompt(_prompt) {}

  //------------------------------------------------------------------------
  //Handle a command
  void handle(string cmd) { registry.handle(cmd, sin, sout); }

  //------------------------------------------------------------------------
  //Read a line into given string
  //Returns false if ctrl-D read
  bool readline(string& line);

  //------------------------------------------------------------------------
  //Run command line
  //Returns when exited
  void run();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CLI_H



