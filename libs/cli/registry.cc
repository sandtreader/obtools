//==========================================================================
// ObTools::CLI: registry.cc
//
// Command Registry implementation
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-cli.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace CLI {

//------------------------------------------------------------------------
//Handle a command
void Registry::handle(string args, istream& sin, ostream& sout)
{
  // Canonicalise the command:
  //   Strip leading/trailing space
  //   Translate whitespace runs into single spaces
  // Note we don't lowercase here - we let CommandGroup do that because
  // we don't want to lowercase eventual command arguments
  args = Text::canonicalise_space(args);

  // Check there is something left (ignore blank lines)
  if (!args.size()) return;

  // Pass to root command group
  CommandGroup::handle(args, sin, sout);
}

}} // namespaces
