//==========================================================================
// ObTools::CLI: command.cc
//
// Command handling implementation
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-cli.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace CLI {

//------------------------------------------------------------------------
//Handle a command
void Command::handle(string args, istream& sin, ostream& sout)
{ 
  // Check for usage present and either no args or "?" or "help"
  if (usage.size() 
   && (!args.size() || args=="?" || Text::tolower(args)=="help"))
    show_usage(sout);
  else
    handler->handle(args, sin, sout); 
}

//------------------------------------------------------------------------
//Show command usage
void Command::show_usage(ostream& sout)
{
  sout << "Usage: " << usage << endl;
}

}} // namespaces
