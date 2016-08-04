//==========================================================================
// ObTools::CLI: cli.cc
//
// Command-line implementation
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-cli.h"
#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace CLI {

//------------------------------------------------------------------------
//Read a line into given string
//Returns false if ctrl-D read
//May be overridden for (e.g.) network read
bool CommandLine::readline(string& line)
{
  line.erase();
  sout << prompt;

  while (!!sin)
  {
    char c=0;
    sin.get(c);

    switch (c)
    {
      case 0:  // EOF
      case 4:  // ctrl-D
        return false;

      case '\r':
      case '\n':
        if (line.size())
          return true;
        else
          sout << prompt;
        break;

      default:
        line+=c;
    }
  }

  return false;
}

//------------------------------------------------------------------------
//Run command line
//Returns when exited
void CommandLine::run()
{
  string line;
  while (readline(line)) handle(line);
}


}} // namespaces
