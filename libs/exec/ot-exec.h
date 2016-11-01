//==========================================================================
// ObTools::Exec: ot-exec.h
//
// Public definitions for ObTools::Exec
//
// Helper to spawn sub-processes
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_EXEC_H
#define __OBTOOLS_EXEC_H

#include "ot-log.h"

namespace ObTools { namespace Exec {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Command to be executed
class Command
{
  vector<string> args;  // [0] is the command path

 public:
  //------------------------------------------------------------------------
  // Constructor from a single command - splits arguments
  Command(const string& command_with_args);

  //------------------------------------------------------------------------
  // Constructor from already split arguments - args[0] is the executable
  Command(const vector<string>& _args): args(_args) {}

  //--------------------------------------------------------------------------
  // Execute a command, passing the given input as stdin and capturing stdout
  // as output_p.  stderr is captured to Log::Error
  // Returns whether the command ran OK
  bool execute(const string& input, string& output_p);

  //--------------------------------------------------------------------------
  // As above, but ignoring input
  bool execute(string& output_p)
  { return execute("", output_p); }

  //--------------------------------------------------------------------------
  // As above, but no input or output
  bool execute()
  { string output; return execute("", output); }
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_EXEC_H
