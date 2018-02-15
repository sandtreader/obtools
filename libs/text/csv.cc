//==========================================================================
// ObTools::Text: csv.cc
//
// Comma (or anything) Separated Variable reader
// Implements RFC4180 but does not allow newlines in fields
// Spaces are treated like any other character and are not stripped
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Read a line of CSV into the given strings
// Won't fail, will try to fix up errors
void CSV::read_line(const string& line, vector<string>& vars)
{
  if (line.empty()) return;  // special case to avoid always adding empty var
  string var;
  bool in_quote{false};
  bool escaped{false};
  for(auto c: line)
  {
    if (escaped)
    {
      escaped = false;
      if (c=='"') // doubled quote
      {
        var+=c;
        continue;
      }

      // Not another quote so leave quoting and use normally
      in_quote = false;
    }

    if (c==sep && !in_quote)  // Non-quoted separator
    {
      vars.push_back(var);
      var.clear();
    }
    else if (c=='"')
    {
      if (in_quote)
        escaped = true;  // Need lookahead to see if it is another quote or end
      else
        in_quote = true;
    }
    else var+=c;
  }

  vars.push_back(var);  // Always use remnant, even if empty
}

//--------------------------------------------------------------------------
// Read multiline CSV into a vector of vector of vars
// Won't fail, will try to fix up errors
void CSV::read(const string& text, vector<vector<string> >& data,
               bool skip_header)
{
  // Split into lines - note keeping blank lines
  vector<string> lines = split_lines(text);
  for(const auto& line: lines)
  {
    if (skip_header) { skip_header=false; continue; }
    vector<string> vars;
    read_line(line, vars);
    data.push_back(vars);
  }
}

}} // namespaces
